# C++ GAPIC Surface Design Review

*Authors: michaelbausor@google.com, vam@google.com, dovs@google.com*

Objective
=========

Describe the design of the new C++ GAPIC generated surface for approval
by the ACTools and Cloud C++ teams. This document covers the **initial**
design of the **generated client surface**. See [`GENERATOR.md`](GENERATOR.md) for a description of the generator.

As the generator matures to beta and eventually GA, we expect that
decisions in this document may be revisited - especially decisions about which GAPIC features to support. This document is intended to describe the requirements for the initial, **alpha-quality** generated surface only, and as such we prefer to exclude features now, with the possibility of introducing them in future.

Background
==========

ACTools creates client library generators for multiple languages, and is now adding support for C++.

Design
======

Compatibility
-------------

We will support the google-cloud-cpp repository as our first and primary customer. Therefore, our generated code will aim to support C++11 along with the [*same set*](https://github.com/googleapis/google-cloud-cpp/tree/master#requirements) of compilers, build tools, dependency versions, and platforms.

Generated clients will **\*not\*** throw exceptions, making them compatible with code that is compiled without exception support.

Dependencies
------------

The generated clients will depend on gax-cpp, which is the runtime library that supports the generated code. This library is also owned and developed by ACTools, and exists primarily to support the generated code.

The generated clients will also depend on protobuf, gRPC and absl. We will attempt to minimize the exposure of gRPC and absl in our public interfaces. Cases where these dependencies may need to be exposed directly to users are:
- `absl::StatusOr<T>`, which we will use as a return type once it becomes available.

The GAPIC stub is intended to hide `gRPC::Status` and `gRPC::ClientContext` behind gax owned types.

Convergence
-----------

We intend to make the generator available inside google3, and allow generated code to be consumed inside google3 via blaze rules. To that end, both the generator and the generated code will follow [*Google C++ style*](https://google.github.io/styleguide/cppguide.html), and attempt to minimize external dependencies that could prevent easy integration into google3.

Generated Clients
-----------------

Each protobuf service definition in the API will be mapped to a client class. Each client class will have a "`<service>_client.gapic.h`" and a "`<service>_client.gapic.cc`" file. Users will need to include the "`<service>_client.gapic.h`" file. So APIs defining multiple services will generate a separate client class for each service. If a single proto file contains multiple services, we will generate a client class (and corresponding ".gapic.h" and ".gapic.cc" file) for each service.

The generated client class will be similar in many respects to the existing `bigtable::noex::Table` or `bigtable::noex::InstanceAdmin` classes in google-cloud-cpp:
-   It will need to be passed a Stub object at construction time (this will be the rough equivalent of `bigtable::DataClient` or `bigtable::InstanceAdminClient`)
-   It can optionally be passed Policy objects that will configure retries, backoff, metadata policy.
-   It will be moveable but (unlike InstanceAdmin class) not copy constructible.
    -   The internal Stub will be held as a `shared_ptr`
    -   Other internal state will be held by `unique_ptrs`
-   The Stub will be an interface that provides a thin wrapper around the generated grpc StubInterface for the service. Because it is an interface, it will be:
    -   Mockable for testing
    -   Possible place for injecting stub/channel pooling behaviour
    -   Possible in future to implement support for non-gRPC transports
- The design of the stub is detailed in a separate document.

Example client:
```cpp
class GreeterClient {
 public:
  // ...
  GreeterClient(std::shared_ptr<GreeterStub> stub);
  GreeterClient(std::shared_ptr<GreeterStub> stub, Policies&&... policies);
  // ...
 private:
  std::shared_ptr<GreeterStub> stub_;
  std::unique_ptr<RPCRetryPolicy> rpc_retry_policy_;
  // ...
};

std::shared_ptr<GreeterStub> CreateGreeterStub(/*opts*/);
```


Example usage:

```cpp
GreeterClient client(CreateGreeterStub());
```

Each client will have at least one method for each rpc defined in the service, which will have the same name as the rpc (with correct casing).

### Client Policies

We will use Policy objects to configure clients at construction time, directly similar to [*google-cloud-cpp Bigtable*](https://github.com/googleapis/google-cloud-cpp/blob/master/google/cloud/bigtable/instance_admin.h#L55). We will define Policy objects very similar to e.g. [*RPCRetryPolicy*](https://github.com/googleapis/google-cloud-cpp/blob/master/google/cloud/bigtable/rpc_retry_policy.h#L64), with one change to the signatures of the Setup and ShouldRetry (renamed from OnFailure) methods:

```cpp
class RPCRetryPolicy {
 public:
  virtual void PrepareContext(gax::CallContext& context) const = 0;
  virtual bool ShouldRetry(gax::CallContext& context, gax::Status const& status) = 0;
};
```

This change is to:
- Avoid exposing `grpc::ClientContext` and `grpc::Status` in the policy interface.
- Provide additional information about the RPC to the policy object via `gax::CallContext`. This allows:
    -  Default retry behaviour to differ between idempotent and non-idempotent methods (by providing info about idempotency via `gax::CallContext`)
    -   Users can implement retry/backoff policies that differentiate per-method (by using method name info available in `gax::CallContext`) if needed.

In addition to RPCRetryPolicy, we will support BackoffPolicy with similar modifications.

### Auth

The client will use `grpc::GoogleDefaultCredentials()` by default. We will
allow users to configure auth in the client constructor.

### Transport and generated GAPIC Stub

The client will use gRPC as the default transport. While we may want to
add support for additional transports in future, at this time we do not
have a concrete plan to support them. Indeed, if grpc-fallback support
is required, it may be gRPC that provides that support rather than GAPIC.

### Unary Methods

**Alpha**: we will generate a single synchronous method that accepts a
request and returns a `gax::StatusOr<Response>`.
```cpp
using pb = google::example::library::v1;
gax::StatusOr<pb::Book> CreateBook(pb::CreateBookRequest const& request);
```

**Post alpha**: design support for method overloads to allow per-call
configuration by users, particularly for:

-   Retries

-   Extracting gRPC metadata

**Post alpha**: Expose async versions of unary methods. Two possible alternatives that we would consider: returning `std::future`, or accepting `std::function` (which is used by the gRPC experimental async interface).

The preferred interface is returning a future for the following reasons:
- Futures are composable using monads, which meshes well with `StatusOr`.
- Futures allow `select` like design patterns and polling.

```cpp
// Example using std::future
std::future<gax::StatusOr<pb::Book>> AsyncCreateBook(pb::CreateBookRequest const& request);

// Example mirroring gRPC experimental async interface
void AsyncCreateBook(pb::CreateBookRequest const& request
                     std::function<void(gax::StatusOr<pb::Book>)>);
```
**Post alpha**: Generate any additional method signatures configured in proto annotations as overloads. These move the responsibility of interacting with protobuf from the user to the client method for commonly set message fields. The information necessary to generate these variants comes in via proto annotations.

### Paginated Methods

See [`PAGINATION.md`](PAGINATION.md) for a detailed design of paginated methods.

### LRO

See [`LRO_DESIGN.md`](LRO_DESIGN.md) for a detailed design of long-running methods.

### Streaming Methods

**Alpha**: we will generate a single method that matches the signature of the gRPC streaming interface.

**Post alpha**: potentially design support wrappers around streaming methods. For example, expose server streaming methods as iterators as per [*ReadRows*](https://github.com/googleapis/google-cloud-cpp/blob/master/google/cloud/bigtable/row_reader.h) from google-cloud-cpp.

### Retries

**Alpha:**

The client will default to the following retry behaviour:
- For **idempotent unary** methods (google.api.http annotation is GET):   - Retry codes: UNAVAILABLE, ABORTED, UNKNOWN
    - Backoff: Reasonable defaults. Follow [*implementation*](https://github.com/googleapis/google-cloud-cpp/blob/master/google/cloud/bigtable/rpc_backoff_policy.h#L44) in google-cloud-cpp.
- For **non-idempotent unary** methods (google.api.http annotation missing or not GET):
    - ~~Retry codes: UNAVAILABLE~~
    - ~~Backoff: Reasonable defaults. Follow [*implementation*](https://github.com/googleapis/google-cloud-cpp/blob/master/google/cloud/bigtable/rpc_backoff_policy.h#L44) in google-cloud-cpp.~~
    -   No retries
-   For **streaming** methods:
    -   No retries

We will allow users to specify retry and backoff policies in the `RetryStub` constructor and DefaultStub factory.

**Post alpha**: Add support for per-call retry and backoff
configuration.

Post-alpha
----------

The generated clients will support the following GAPIC features, but we intend to delay their implementation until **after** the surface reaches an alpha state:
- Routing headers
- Method flattening
- In-code samples

Unsupported features
--------------------

At this time, we do **not** intend to support the following GAPIC
features:
- Resource names

Packaging generation
--------------------

**Alpha**: No packaging generation.

**Post Alpha**: We will generate minimal cmake files to allow generated clients to be compiled, and to run any generated tests. We will **\*not\*** try to generate complex packaging suitable for publishing generated clients. Allow packaging generation to be controlled via protoc plugin arguments.

Testing Plan
============

**Alpha**: We will rely on tests in the generator (e.g. unit tests, baseline tests) to ensure correctness of generated output. When generating via bazel rule (instead of using the plugin with protoc), we will support compiling the generated client as a subsequent build step.

**Post alpha**: We will generate unit and smoke tests. When generating via bazel rule, we will support running generated unit tests as a subsequent build step (after compilation). The generated minimal cmake packaging will enable running unit and smoke tests.

**Post alpha**: Full integration with the [*GAPICShowcase*](https://github.com/googleapis/gapic-showcase) test suite.
- Note: GAPIC Showcase is ACTools' test API and in-memory server, designed for testing the full feature set of generated client libraries.

References
==========
-   [*https://github.com/googleapis/google-cloud-cpp*](https://github.com/googleapis/google-cloud-cpp)

# 'C++ GAPIC LongRunning Operation Design Doc' #

*dovs@google.com*

## Overview
========

This document is a proposal for how C++ GAPIC generated client libraries will interact with API methods that return google.longrunning.Operation. It covers the 'synchronous' interface exposed to users of the client library and the underlying architecture and mechanisms.

This design is heavily based on the `TableAdmin::SnapshotTable` method from the BigTable client library.

**Note:** although the user visible surface in this document is asynchronous, a distinction is made between it and the future asynchronous method variants generated clients will provide for all methods. These asynchronous methods will rely on a user-injected completion queue that depends on user defined execution mechanisms, e.g. a thread pool or executor. Many of the details required for asynchronous methods are not finalized and fall outside the scope of this document.

In addition to the 'synchronous' long running operation methods described in this doc, there will also be corresponding 'asynchronous' long running operation methods that use the same infrastructure as other asynchronous client methods.

## Background
==========

A common pattern for API methods that may take a long time to complete (where 'long time' can be anywhere from seconds to days) is to return an instance of `google.longrunning.Operation`, which contains a unique, server provided name; a flag indicating whether the operation is completed; and a union of the concrete return message and an error status.

A separate Operations service exposes methods that use operation names. The most important methods for the purposes of this document is GetOperation, which takes a message using the name of an existing, uncompleted operation and returns a new `google::longrunning::Operation` object with the same name. This new operation may be in a completed state, in which case its result is usable by the client.

These semantics mean that the normal method of client interaction is to poll the operations service until the desired operation has completed successfully or failed.

## Exposed primitive types

## PollingPolicy interface

The PollingPolicy interface is a primitive used to construct a polling loop.

### Interface:
```cpp
class PollingPolicy {
 public:
  virtual ~PollingPolicy() = default;
  virtual std::unique_ptr<PollingPolicy>; clone() = 0;
  virtual void Setup(gax::CallContext&) = 0;
  virtual bool IsExhausted() = 0;
  virtual bool IsPermanentFailure(gax::Status const&) = 0;
  virtual std::chrono::milliseconds WaitPeriod() = 0;
};
```
------------------------------------------------------------

### Default implementation:

```cpp
template<typename Retry = LimitedTimeRetryPeriod,
         typename Backoff = ExponentialBackoffPolicy>
class GenericPollingPolicy : public PollingPolicy {
 public:
  GenericPollingPolicy(Retry retry, Backoff backoff);
  // May provide additional constructors from a defaults-wrapping dumb struct.
  // Wraps methods from retry and backoff to fulfil interface.
  };
```
------------------------------------------------------------------------------

## `gax::Operation<Response, Metadata>` object
-----------------------------------------------

This interface provides a thin wrapper over the Operation message type with these addenda:

-   The response and metadata types are strong; the types are obtained from proto annotations and reified by the GAPIC generator.

-   Calls to the Operations service, e.g. GetOperation and DeleteOperation, are wrapped by methods of gax::Operation.

The Operation class is move constructable but not copy constructable or default constructable. It has at least one constructor that is hidden from the user and is used by internal code.

### `gax::Operation` interface:

```cpp
template<typename ResultType, typename MetadataType>
class Operation final { 
 public:
 Operation(std::shared_ptr<OperationsStub> stub, google::longrunning::Operation op);
 Operation(Operation&& other) = default; 
 Operation() = delete;
 Operation(Operation const&) = delete;
 
 gax::StatusOr<ResultType> 
 RunPollLoop(PollingPolicy const& polling_policy = DefaultPollingPolicy(), 
             std::function<void(MetadataType const&)> metadata_func = nullptr); 
   
 std::string const& Name() const; // Returns the name assigned by the remote service.
 
 bool Done() const; // Indicates whether the operation has completed.
 void Cancel(); // Best effort attempt to cancel the remote operation.
 void Delete(); // Informs the service that the client is no longer
                // interested in the result.
   
 MetadataType const& Metadata() const; // Returns a reference to the metadata
                                       // from the most recent poll.
                                       // **Note:** this does not currently consider
                                       // the possibility that the metadata failed
                                       // to update correctly, either due to type
                                       // mismatch or other reasons.

 gax::Status Update(); // Queries the service and gets 
                       // the most recent result of the operation.
 
 gax::StatusOr<ResultType> Result(); // Returns the end result of the operation.
                                     // Calling Result() if !Operation::done()
                                     // returns a Status with code kUnknown. 
 };
 static_assert(std::is_move_constructable<Operation>::value);
 static_assert(!std::is_default_constructable<Operation>::value);
 static_assert(!std::is_copy_constructable<Operation>::value);
```

## `gax::OperationsStub` abstract class
----------------------------------

If the client has at least one LRO, its generated GAPIC stub becomes a child of the OperationsStub abstract class. The `gax::OperationsStub` class is abstract and uninstantiable but provides default, UNIMPLEMENTED status returning definitions for GetOperation, DeleteOperation, and CancelOperation.

The GAPIC generator provides overrides for these methods in the generated default stub class.

### Proposed OperationsStub:
```cpp
namespace gax {
class OperationsStub {
  virtual ~OperationsStub() = 0;
 
  virtual grpc::Status
  GetOperation(gax::CallContext& context,
               google::longrunning::GetOpertaionRequest const &request,
               google::longrunning::Operation *response) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                        "GetOperation is not implemented");
  }
  
  virtual grpc::Status
  DeleteOperation(gax::CallContext& context,
                  google::longrunning::DeleteOpertaionRequest const &request,
                  google::longrunning::Empty *response) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                        "DeleteOperation is not implemented");
  }
  
  virtual grpc::Status
  
  CancelOperation(gax::CallContext& context,
                  google::longrunning::CancelOpertaionRequest const &request,
                  google::longrunning::Empty *response) {
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED,
                        "CancelOperation is not implemented");
  }
};


inline OperationsStub::~OperationsStub() {}
  
}
```

### Changes to generated GAPIC abstract client:

```
class LibraryServiceStub : public OperationsStub {
  // No changes in interface or method implementations. 
};
```

### Changes to generated GAPIC default concrete client:
  
```
class DefaultLibraryServiceStub : public LibraryServiceStub {
  grpc::Status
  GetOperation(gax::CallContext& context,
               google::longrunning::GetOpertaionRequest const &request,
               google::longrunning::Operation *response) override {
  // Do whatever the other generated overrides do.
  }
};
```

## Changes to existing user visible surfaces
=========================================

### GAPIC generated client Stub
---------------------------

The generated abstract Stub class for each GAPIC client will be modified to extend the `gax::OperationsStub` abstract class and will no longer need to provide its own pure virtual destructor.

For clients that have LRO methods, the default concrete Stub class will be augmented to provide `GetOperation`, `CreateOperation`, and `DeleteOperation` methods that issue gRPC calls in a manner similar to other overrides.

### GAPIC generated client
----------------------

The GAPIC client will gain two methods for interacting with the long-running API.

The GAPIC client will also gain a factory function template that returns instances of `gax::Operation`. This function will take a name describing an in-flight operation and an optional polling policy. This may be useful if a user initiates operations from one process but wants to poll for operation completion in another process.

Example:

```cpp
class LibraryService final {
  public:
  
  // Other methods
  // Proposed implementation elided.
  
  gax::StatusOr<gax::Operation<Book, GetBigBookMetadata>>
  GetBigBookOperation(Book const& request,
  PollingPolicy const& pp = DefaultPollingPolicy());
  
  // Proposed implementation elided.
  std::future<gax::StatusOr<Book>>
  GetBigBookFuture(Book const& request,
  PollingPolicy const& pp = DefaultPollingPolicy());
  
  template<typename ResultType, typename MetadataType>
  Operation<ResultType, MetadataType>
  TryOperation(std::string name, PollingPolicy const& pp = DefaultPollingPolicy()) {
    google::longrunning::Operation op;
    op.set_name(name);
    return Operation<ResultType, MetadataType>(operations_stub_, std::move(op), pp);
  }
  };
```

### User profiles and corresponding interfaces
==========================================

The design for client methods that use `google::longrunning::Operation`
recognizes three broad user profiles:
-   Users who want the simplest, most direct interface, as close as possible to a normal synchronous method call.
-   Users who want complete control over low level details like polling and metadata.
-   Middle-ground users who want the client to handle polling, do not want to block waiting for the result, and may want to use operation metadata.

The example API method used below is LibraryService.GetBigBook, with concrete response type Book and concrete metadata type GetBigBookMetadata.

```protobuf
service LibraryService {
  rpc GetBigBook(GetBigBookRequest) returns google.longrunning.Operation {
  option (google.longrunning.operation_info) = {
    response_type: "Book"
    metadata_type: "GetBigBookMetadata"
    };
  }
}
```

### Exposed methods from the client:

```
gax::StatusOr<gax::Operation<Book, GetBigBookMetadata>>
  GetBigBookOperation(GetBookRequest const&);
  google::cloud::future<gax::StatusOr<Book>>
  GetBigBook(GetBookRequest const&,
  std::function<void(GetBigBookMetadata const&)> = nullptr);
```

### Raw `gax::Operation`
------------------

These users want to implement their own custom polling logic or manage the concurrency of the operation on their own. To allow this, we allow them to operate on `gax::Operation` directly.

The GAPIC client will provide method variants that return `gax::StatusOr<gax::Operation<ResultType, MetadataType>>`.

#### Example interface:

 ```cpp
 class LibraryClient final {
  // other client methods and attributes
  gax::StatusOr<Operation<Book, GetBigBookMetadata>> GetBigBookOperation(GetBookRequest const&);
  // other client methods and attributes
  };
```

#### Example usage:

```cpp
auto res = client.GetBigBookOperation(request);
  
  if(!res) {
  // do something
  return;
  }
  
  gax::Operation<Book, GetBigBookMetadata> operation = *std::move(res);
  operation.Update();
  
  if (!operation.Done()) {
    if (justOnce) {
      operation.Cancel();
      return;
    } else if (dontCare) {
      operation.Delete();
      return;
  }
  
  operation.Update();
  GetBigBookMetadata const& metadata = operation.Metadata();
  std::cout << "Operation metadata =" << metadata << std::endl;
  }
  
  // Assume that operation is done now for the sake of this example.
  gax::StatusOr<Book> result = std::move(operation).Result();
```

#### Possible implementation of GetBigBookOperation:

```cpp
gax::StatusOr<gax::Operation<Book, GetBigBookMetadata>>
  GetBigBookOperation(GetBookRequest const& request,
  PollingPolicy const& polling_policy = DefaultPollingPolicy()) {
  google::longrunning::Operation op;
  // Engage in normal, synchronous rpc retry and backoff using stub_->GetBigBook()
  // Details are elided.
  return gax::Operation<Book, GetBigBookMetadata>(operations_stub_,
                                                  std::move(op),
                                                  polling_policy);
  }
```

### `std::future<StatusOr<Response>>`

The users who chose to operate on `std::future<StatusOr<Response>>` do not want to perform their own polling but also do not want to block the current thread unconditionally. They may also wish to interact with operation metadata, possibly to update a progress meter or for logging purposes.

#### Possible implementation:

  
```cpp
std::future<gax::StatusOr<Book>> GetBigBook(
  GetBookRequest const& request,
  PollingPolicy const& polling_policy = DefaultPollingPolicy(),
  std::function<void(GetBigBookMetadata const &)> handle_metadata = nullptr) {
  StatusOr<gax::Operation<Book, GetBigBookMetadata>> opStat = GetBigBookOperation(request, polling_policy);
  
  if (!opStat) {
    std::promise<gax::StatusOr<Book>> status_promise;
    status_promise.set_value(std::move(opStat).Status());
    return status_promise.get_future();  
  }
  
  auto launch_policy = bool(handle_metadata) ? std::launch::async : std::launch::deferred;
  
  // **Note:** the capture semantics below are almost certainly wrong.
  // Operation is not copyable, and C++11 doesn't provide native move capture.
  // There are workarounds: one is to make the copy constructor private and just
  // copy it here.
  return std::async(launch_policy, [opStat, handle_metadata]() {
    return opStat.RunPollLoop(polling_policy, std::move(handle_metadata));
  });
  
  }
```

#### Example usage:

  
```cpp
  std::future<gax::StatusOr<Book>> fut = client.GetBigBookFuture(request);
  
  // Do other useful work
  auto result = fut.wait();
  
  std::atomic_uint8_t percent_complete;
  
  std::future<gax::StatusOr<Book>> fut2 = client.GetBigBookFuture(request2,
    // This lambda is an optional parameter.
   [&percent_complete] (GetBigBookMetadata const& metadata){
   percent_complete.store(metadata.progress_percent()); });

  // Do other work, use percent_update in another thread to update a progress bar
  auto result2 = fut.wait();
```

### Launch policy

Once the initial rpc is invoked, the operation runs on the backend service without requiring additional work by the client. The only purpose of the polling loop is to check for a result and optionally to view operational metadata. As a result, in the case where metadata manipulation is not required, the deferred launch policy conserves system resources and preserves semantics.

If the user needs to manipulate operation metadata, the async launch policy is preferable so that metadata updates are continual.

See [*https://en.cppreference.com/w/cpp/thread/launch*](https://en.cppreference.com/w/cpp/thread/launch) and Effective Modern C++: Item 36[^2] for details on launch policy.

### StatusOr<Response>
------------------------

This interface is isomorphic to other unary, synchronous API methods. The calling thread blocks waiting for the result, which may take a long time to be available.

E.g.

```cpp
  StatusOr<Book> result = client.GetBigBookSync(request);
```

This is functionally equivalent to, and will probably be implemented with,
```cpp
  StatusOr<Book> GetBigBookSync(GetBookRequest const& request) {
  auto fut = client.GetBigBook(request);
  return fut.wait();
  }
```

**Note:** Given the extreme simplicity of the implementation, this interface may not be implemented by GAPIC but instead left as an exercise to the user.

#### Detailed design
===============

### `gax::Operation`
--------------

```cpp
template <typename ResultType, typename MetadataType>
class Operation final {
public:

// The real constructor isn't really for general use, but can't think of a
// good way to hide it and let generated clients call the constructor.

Operation(std::shared_ptr<OperationsStub> stub,
          google::longrunning::Operation op,
          PollingPolicy const &polling_policy)
  : stub_(std::move(stub)), op_(std::move(op)),
    polling_policy_(polling_policy.clone()) {}

Operation() = delete;
Operation(Operation const &) = delete;
Operation(Operation &&) = default;

bool Done() { return op_.done(); }

gax::StatusOr<ResultType> Result() {
  if (!op_.done()) {
    return gax::Status(gax::StatusCode::kUnknown,
                       "operation has not completed=" + op_.name());
  }

  if (op_.has_error()) {
    return gax::Status(static_cast<gax::StatusCode>(op_.error().code()),
                       op_.error().code().message());
  }

  auto const& any = op_.response();
  if (!any.Is<ResultType>()) {
    return gax::Status(gax::StatusCode::kUnknown,
                       "invalid result in operation=" + operation.name());
  }

  ResultType result;
  any.UnpackTo<ResultType>(&result);
  return result;
}

gax::Status Update() {
  if (!done()) {
    google::longrunning::GetOperationRequest getOpReq;
    getOpReq.set_name(op_.name());
    gax::CallContext context;
    return stub_->GetOperation(&context, getOpReq, &op_);
    } else if (op_.has_error()) {
      return gax::Status(static_cast<gax::StatusCode>(op_.error().code()),
                         op_.error().message());
    } else {
    // Default ctor is kOk
    return gax::Status;
    }
}

MetadataType const& Metadata() const {
  auto const& any = op_.metadata();
  if (any.Is<MetadataType>()) {
    any.UnpackTo<MetadataType>(&md_);
  } else {
    md_ = MetadataType();
  }

  return md_;
}

void Cancel() {
  google::longrunning::CancelOperationRequest canOpReq;
  google::protobuf::Empty empty;
  canOpReq.set_name(op_.name());
  gax::CallContext context;
  stub_->CancelOperation(&context, canOpReq, &empty);
}

void Delete() {
  google::longrunning::DeleteOperationRequest delOpReq;
  google::protobuf::Empty empty;
  delOpReq.set_name(op_.name());
  gax::CallContext context;
  stub_->DeleteOperation(&context, delOpReq, &empty);
}
```

### Polling loop

```cpp
gax::StatusOr<ResponseType>
RunPollLoop(PollingPolicy const& polling_policy = DefaultPollingPolicy(),
            std::function<void(MetadataType const&)> metadata_func = nullptr) {
  auto pp = polling_policy.clone();  
  while (true) {
    auto status = Update(*pp);
    if (metadata_func) {
      metadata_func(Metadata());
    }
  
  if (pp->IsPermanentFailure(status)) {
    return status;
  }
  
  if (Done()) {
    return Result();
  }
  
  if (pp->IsExhausted()) {
    return gax::Status(gax::StatusCode::kDeadlineExceded,
                       "polling timed out for operation=" + Name());
  }
  
  auto delay = pp->WaitPeriod();
  std::this_thread::sleep_for(delay);
  }
  
}
```

Open questions
==============

Customization and exposure of PollingPolicy
-------------------------------------------

The expected wait time for an operation to complete is extremely method specific and may vary wildly even for a single method. An auto-generated default, either client-wide or method specific, is unlikely to be useful. Similarly, the utility of an injected client-wide PollingPolicy is limited.

### Initial API call error vs. polling error

Interacting with raw gax::Operation gives potentially useful information on failure: whether the failure happened during the initial call to the API, or whether it happened during polling. This nuance is lost if the user wants to use the future returning variant provided by the GAPIC client.

Is it reasonable to elide this distinction in the future-returning interface?

Design Alternatives
===================

Standalone OperationsStub
-------------------------

This was the original design proposal but was superceded by the `OperationsStub` base class in light of the evolving design of `gax::CallContext` and implementing method retry via decoration.

The `OperationsStub` type is independent of the GAPIC stub. It is stored as an additional attribute of LRO owning clients. The base OperationsStub class is instantiable and returns UNIMPLEMENTED statuses for all its methods. Gax also owns a default implementation that issues gRPC calls.

Instead of making `OperationsStub` an injected parameter to the GAPIC client constructor, the GAPIC stub is given a factory method that returns `std::unique_ptr<OperationsStub>`. This lets users who subclass the GAPIC stub to also customize OperationsStub while giving them the option of relying on the default implementation.

This alternative was abandoned because it adds yet another customization point, which is user-unfriendly, and the GAPIC stub factory method injection mechanism was admittedly a hack.

The status quo (OperationsStub is a base class for GAPIC stubs) has the following tradeoffs verses a standalone type:

**Pros:**
-   Solves injection problem cleanly and removes the need for a factory method.
-   Gives the user a single point of customization instead of two. This is attractive if the user wants to provide custom gGRPC stub semantics (e.g. channel affinity), use an alternative transport (e.g. Stubby or json/REST), or add decorators for logging or other purposes.
-   Conceptually cleaner: the fact that the Operations service is distinct from the Client service is an implementation detail.
-   The gax::Operation class remains agnostic to the concrete type of the stub passed to it in the constructor.
    -   A previous design alternative added another template parameter to Operation which was used to qualify the type of the stub.

**Cons:**
-   Tightly couples the Operations methods to the same channel and gRPC stubs (or similar for alternative transports) as the GAPIC stub.
    -   This may make it difficult to interact with the Operations service for an API if it ever has a different URL from the primary service.
-   Decoration is less important for Operations methods.=
    -   Logging and OpenCensus support make sense.
    -   Duplicate request removal, client side caching, and customized retry do not.

References
==========

[*https://cloud.google.com/apis/design/design\_patterns*](https://cloud.google.com/apis/design/design_patterns)
[*https://github.com/googleapis/googleapis/blob/master/google/longrunning/operations.proto*](https://github.com/googleapis/googleapis/blob/master/google/longrunning/operations.proto)
[*https://en.cppreference.com/w/cpp/thread/future*](https://en.cppreference.com/w/cpp/thread/future)

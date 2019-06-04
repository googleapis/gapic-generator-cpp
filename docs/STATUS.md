# C++ GAPIC State of the World #
dovs@google.com

The C++ GAPIC project is being shelved/handed off as of 2019-05-28. This document is intended to be an infodump describing the current status of the project, intended design decisions and philosophies, lacunae, next steps, and anything else that may be relevant to those resurrecting the project at some point in the future.

**NOTE:** the design docs listed here are maintained on a best effort status. They are meant to illuminate design goals, tradeoffs, and plans, but are NOT maintained as live user or developer documentation. If a design in the code and the docs diverge, the code is authoritative.

## Current Capabilities ##

### Generator ###

The generator can read in a correctly formatted service proto file and emit a compilable surface. It can be invoked in a standalone manner or as a proto plugin.
There is a bazel rule that builds the generator.
There is a bazel rule that uses the generator to create and compile client library code. It must be passed the label for the service's proto library 'with info' and the labels for the service's `cc_grpc` and `cc_proto targets` as dependencies.
E.g.:
```protobuf
load("//rules_gapic/cpp:cc_gapic.bzl", "cc_gapic_srcjar", "cc_gapic_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@com_google_api_codegen//rules_gapic:gapic.bzl", "proto_library_with_info")

proto_library(
    name = "library_proto",
    srcs = ["library.proto"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_googleapis//google/api:client_proto"],
)

proto_library_with_info(
    name = "library_proto_with_info",
    deps = [":library_proto"],
)

cc_proto_library(
    name = "library_cc_proto",
    visibility = ["//visibility:public"],
    deps = [":library_proto"],
)

cc_grpc_library(
    name = "library_cc_grpc",
    srcs = [":library_proto"],
    grpc_only = True,
    deps = [":library_cc_proto"],
)

cc_gapic_library(
    name = "library_cc_gapic",
    src = ":library_proto_with_info",
    package = "google.example.library.v1",
    visibility = ["//visibility:public"],
    deps = [
        ":library_cc_grpc",
        ":library_cc_proto",
    ],
)
```
The two headers for the generated surface, `\*.gapic.h` and `\*_stub.gapic.h`, are exported by the target and are visible at sane include paths in dependent code.

### Generated Client ###

There are two factory functions that return a GAPIC stub; both return a retry stub decorating a 'direct' gRPC invoking stub.
Assuming the service proto is annotated correctly and credentials have been properly set in the environment, synchronous client methods for unary API calls are generated and can be invoked.

### Gax ###

Helper types and library code exist that support the following features:
* Paginated methods
* Long running operations
* Idempotent method retry
* Custom retry and backoff policies
* Setting custom per-call gRPC metadata

## Current Limitations ##

### Generator ###

* The generator does not validate the service proto file and does not generate the errors expected by the config validator.
* The generator does not detect long running operations.
* The generator does not detect paginated methods.
* The generator does not create self-contained, compilable samples.
* The generator does not create tests for any part of the generated client.
* The generator does not create standalone Bazel or CMake build files for compiling the client.
* The generator does not annotate the generated client with doxygen comments.
* The generator currently assumes no methods are idempotent.

### Generated Client ###

* There is no mechanism for configuring the service endpoint via the generated GAPIC stub factory functions.
* Long running operations:
    * Clients with long running methods do not have `gax::OperationsClient` returning factory methods.
    * Client methods that correspond to long running API methods do not return `gax::Operation` instances.
    * There is no `gax::future<gax::StatusOr<Response>>` returning method variant.
* No code is generated to support streaming API methods.
* No asynchronous method variants are generated.
* Both the client class and the abstract GAPIC stub (and the hidden, concrete, default GAPIC stub classes) live in the global namespace.
* Minimal support for non-gRPC transports.
* Support for anything besides default credentials and authentication is non-existent.

### Gax ###

* No supporting types or library routines exist supporting asynchronous method variants.
* The intended asynchronous primitives are intended to come from Abseil or to be stolen from the Cloud C++ repository.
* The LRO retry loop is not implemented as it relies on asynchronous primitives not yet in the repository.
* No supporting types or library routines exist supporting streaming methods.
* The PaginatedResponse template class, tying together Pages and PageResult, is unimplemented.

## In progress designs ##

The limitations of the generator require little to no design or discussion: they are straightforward feature implementations and conformance to the standard set by the other microgenerators. The serious design decisions relate to the generated surface and gax-owned helper types and library code.

A tentative, verbal decision was made to support asynchronous method variants by providing client methods that return `gax::future<gax::StatusOr<Response>>`.
The transition of idiomatic asynchronous gRPC from completion-queues to callbacks may adjust this decision, but this is unlikely.

No decision was made on this front, but the idea was raised that, since abstracting gRPC away from the implementation of asynchronous method variants and streaming methods may be non-trivial, these methods may only be supported in a child class of the 'vanilla' generated client. The vanilla client therefore remains transport agnostic without forcing an onerous abstraction.
No design work has been done to support streaming methods.
No design work has been made to explicitly support any transport besides gRPC.

## Misc ##
No benchmarks or leak check tests have been implemented for either gax code or generated client code.

## References ##
[Google Cloud Cpp repository](https://github.com/googleapis/google-cloud-cpp/)

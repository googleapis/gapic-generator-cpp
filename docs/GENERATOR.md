# C++ GAPIC Generator Design Review #

*Authors: michaelbausor@google.com, vam@google.com, dovs@google.com*

Objective
=========

Describe the design of a C++ client library generator. This document covers the design of the **generator**; the [generated surface](SURFACE.md) and GAX-owned primitives are discussed in other docs.


Compatibility
-------------

We will support the google-cloud-cpp repository as our first and primary customer. Because we intend to support users building and running the
GAPIC generator themselves (possibly as part of compiling a project that consumes the generated surface), we will aim to support the [*same set*](https://github.com/googleapis/google-cloud-cpp/tree/master#requirements) of compilers, build tools, dependency versions, and platforms with our generator as are supported by google-cloud-cpp (and by our generated code).

The generator will **\*not\* throw exceptions**, making it compatible with code that is compiled without exception support.

Dependencies
------------

We will attempt to minimize the number of external dependencies taken by the generator. Because the generator will implement the protobuf [*CodeGenerator*](https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/compiler/code_generator.h#L66) interface, we have a dependency on protobuf. We will also take an \*internal only\* dependency on the [*abseil*](https://abseil.io/) library.

Versioning and Publishing
-------------------------

The GAPIC generator will follow semantic versioning. Once the generator is declared GA, we are committing to not breaking users of the generator, **and** not making any breaking changes in the generated surface. We will use Github releases to tag and publish generator versions. Initially we will not support any C++ package managers. However, we will facilitate community contributions so that package manager support is possible in future. 

Building the Generator
----------------------

The generator will be buildable via bazel and cmake, similar to the existing google-cloud-cpp repository. We will use bazel as the source of truth, and generate cmake files from bazel. Example of building using bazel:

```bash
$ git clone https://github.com/googleapis/gapic-generator-cpp
$ cd gapic-generator-cpp
$ bazel build ...
$ bazel test ...
```

Consuming the Generator
-----------------------

### Via bazel

We will provide a `cpp_gapic_library` build rule that will depend on the annotated proto files and output a generated gapic library. This will be similar to the rule that exists for gapic-generator in Java [*here*](https://github.com/googleapis/gapic-generator/blob/master/rules_gapic/java/java_gapic.bzl#L139) or Go [*here*](https://github.com/googleapis/gapic-generator/blob/master/rules_gapic/go/go_gapic.bzl#L98).
See [*go/gapic-bazel-extensions*](https://goto.google.com/gapic-bazel-extensions) for more detail about exposing generation (as well as testing generated libraries and generating packaging) as bazel rules.

### Via protoc plugin

Users can call protoc directly and specify a built gapic-generator-cpp plugin binary. We will defer publishing built binaries that users can download until the generator reaches a post-alpha state, at which time we will re-review.

Implementation
--------------

The GAPIC generator will implement the protobuf [*CodeGenerator*](https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/compiler/code_generator.h#L66) interface. Where possible, all logic will be implemented as part of the [*Generate*](https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/compiler/code_generator.h#L82) method.

The only exception to this is processing metadata annotation information, which may be stored in another protobuf file from the one currently being generated. This will be preprocessed in the [*GenerateAll*](https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/compiler/code_generator.h#L98) method, and made available to the Generate method. If metadata annotation information is inconsistent across files, the generator will exit with an error.

### Generate Method

The generator will make use of the protobuf [*io::Printer*](https://github.com/protocolbuffers/protobuf/blob/master/src/google/protobuf/io/printer.h#L181) object to write files. The printer provides simple templating functionality by accepting a `std::string` template and a set of variables, performs variable substitution on the template, and outputs the result. Unlike more advanced templating languages such as Jinja2, it does not support complex [*control structures*](http://jinja.pocoo.org/docs/2.10/templates/#list-of-control-structures) such as looping. Where loops are necessary (such as over the RPCs in a service) they must be implemented in code. This makes separating the templates from the data model much more difficult than in (for example) gapic-generator-python.

We extract information from the file descriptor and store it in a `std::map<std::string, std::string>` object suitable for use with `io::Printer`. This map is mutable and is passed to methods that iterate over elements of the descriptor, update the map with data from the descriptor, and render templates.

Sample Generation
-----------------

The generator will support in-code method samples as a GA requirement, but we will delay their implementation until post-alpha. The generator will also support standalone samples, implementation timeline TBD but likely prioritized after in-code samples. We will work with the Samplegen team to ensure we can support both use cases, and cover the full range of sample specification.

The google-cloud-cpp repository contains [*standalone samples*](https://github.com/googleapis/google-cloud-cpp/tree/master/google/cloud/bigtable/examples) that are compilable and testable, and that also contain annotations that allow them to be [*linked in to documentation*](https://googleapis.github.io/google-cloud-cpp/0.6.0/bigtable/bigtable-hello-world.html). We can follow this model to support documentation and testing of samples.

Testing and CI
--------------

We will use Kokoro as our CI system. We will integrate with [*GAPIC Showcase*](https://github.com/googleapis/gapic-showcase) to test feature coverage. For example, to use GAPIC Showcase to test paginated methods, we can generate a Showcase API Echo service client and call the [*PagedExpand*](https://github.com/googleapis/gapic-showcase/blob/master/schema/echo.proto#L82) method to verify that our pagination code works as expected. Calls will be made against a local in-memory Showcase API server.

Documentation
-------------

We will use doxygen, [*similar to google-cloud-cpp*](https://googleapis.github.io/google-cloud-cpp/0.6.0/bigtable/index.html), hosted at https://googleapis.github.io/gapic-generator-cpp.

References
==========
- [*https://github.com/googleapis/google-cloud-cpp*](https://github.com/googleapis/google-cloud-cpp)

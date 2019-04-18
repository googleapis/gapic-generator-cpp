// Copyright 2019 Google Inc.  All rights reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <map>
#include <string>

#include "data_model.h"
#include "printer.h"
#include "stub_header_generator.h"
#include <google/protobuf/descriptor.h>

namespace pb = google::protobuf;

namespace google {
namespace api {
namespace codegen {
namespace internal {

std::vector<std::string> BuildClientStubHeaderIncludes(
    pb::ServiceDescriptor const* service) {
  return {
      LocalInclude(absl::StrCat(service->name(), ".pb.h")),
      LocalInclude("grpcpp/client_context.h"), LocalInclude("grpc/status.h"),
  };
}

std::vector<std::string> BuildClientStubHeaderNamespaces(
    pb::ServiceDescriptor const* service) {
  return {};
}

bool GenerateClientStubHeader(pb::ServiceDescriptor const* service,
                              std::map<std::string, std::string> const& vars,
                              Printer& p, std::string* /* error */) {
  auto includes = BuildClientStubHeaderIncludes(service);
  auto namespaces = BuildClientStubHeaderNamespaces(service);

  p->Print(vars,
           "// Generated by the GAPIC C++ plugin.\n"
           "// If you make any local changes, they will be lost.\n"
           "// source: $proto_file_name$\n"
           "#ifndef $stub_header_include_guard_const$\n"
           "#define $stub_header_include_guard_const$\n"
           "\n");

  for (auto const& include : includes) {
    p->Print("#include $include$\n", "include", include);
  }

  for (auto const& nspace : namespaces) {
    p->Print("namespace $namespace$ {\n", "namespace", nspace);
  }

  p->Print("\n");

  // Abstract interface Stub base class
  p->Print(vars,
           "class $stub_class_name$ {\n"
           " public:\n");

  DataModel::PrintMethods(
      service, vars, p,
      "  virtual grpc::Status $method_name$(grpc::ClientContext* context,\n"
      "    $request_object$ const& request,\n"
      "    $response_object$* response);\n"
      "\n",
      NoStreamingPredicate);

  p->Print(vars,
           "  virtual ~$stub_class_name$() = 0;\n"
           "\n"
           "}; // $stub_class_name$\n"
           "\n"
           "std::unique_ptr<$stub_class_name$>\n"
           "Create$stub_class_name$();\n"
           "\n"
           "std::unique_ptr<$stub_class_name$>\n"
           "Create$stub_class_name$(std::shared_ptr<grpc::ChannelCredentials> "
           "creds);\n"
           "\n"
           "#endif // $stub_header_include_guard_const$\n");

  return true;
}
}  // namespace internal
}  // namespace codegen
}  // namespace api
}  // namespace google

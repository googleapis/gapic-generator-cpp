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

#include "client_cc_stub_generator.h"
#include "gapic_utils.h"
#include "printer.h"
#include "src/google/protobuf/descriptor.h"
#include "generator/internal/data_model.h"

namespace pb = google::protobuf;

namespace google {
namespace api {
namespace codegen {
namespace internal {

std::vector<std::string> BuildClientStubCCIncludes(
    pb::ServiceDescriptor const* service) {
  return {LocalInclude(
      absl::StrCat(CamelCaseToSnakeCase(service->name()), "_stub.gapic.h"))};
}

std::vector<std::string> BuildClientStubCCNamespaces(
    pb::ServiceDescriptor const* /* service */) {
  return {};
}

bool GenerateClientStubCC(pb::ServiceDescriptor const* service,
                          std::map<std::string, std::string> const& vars,
                          Printer& p, std::string* /* error */) {
  auto includes = BuildClientStubCCIncludes(service);
  auto namespaces = BuildClientStubCCNamespaces(service);

  p->Print(vars,
           "// Generated by the GAPIC C++ plugin.\n"
           "// If you make any local changes, they will be lost.\n"
           "// source: $proto_file_name$\n");

  for (auto const& include : includes) {
    p->Print("\n#include $include$", "include", include);
  }
  for (auto const& nspace : namespaces) {
    p->Print("\nnamespace $namespace$ {", "namespace", nspace);
  }

  p->Print("\n");

  DataModel::PrintMethods(
      service, vars, p,
      "\n\ngrpc::Status\n$stub_class_name$::$method_name$("
      "grpc::ClientContext*,"
      "\n    $request_object$ const&,"
      "\n    $response_object$*) {"
      "\n    return grpc::Status(grpc::StatusCode::kUnimplemented,"
      "\n        \"$method_name$ not implemented\");\n}",
      NoStreamingPredicate);

  p->Print(vars, "\n\n$stub_class_name$::~$stub_class_name$() {}\n");

  for (auto const& nspace : namespaces) {
    p->Print("\n} // namespace $namespace$", "namespace", nspace);
  }

  return true;
}

}  // namespace internal
}  // namespace codegen
}  // namespace api
}  // namespace google

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

#include <string>
#include <map>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_replace.h"
#include "generator/gapic_generator.h"
#include "internal/gapic_utils.h"
#include "internal/gapic_common_templates.h"
#include "internal/gapic_header_templates.h"

#include "google/api/client.pb.h"

namespace pb = google::protobuf;

namespace google {
namespace api {
namespace codegen {

using Vars = std::map<std::string, std::string>;

void SetServiceVars(
    pb::ServiceDescriptor const* service,
    Vars& vars) {
  vars["class_name"] = service->name();
  vars["proto_file_name"] = service->file()->name();
  vars["header_include_guard_const"] = absl::StrCat(service->name(), "_H_");
  vars["class_comment_block"] = "// TODO: pull in comments";
  vars["grpc_stub_fqn"] = internal::ProtoName2CppName(service->full_name());
  vars["service_endpoint"] = service->options().GetExtension(google::api::default_host);
}

void SetMethodVars(
    pb::MethodDescriptor const* method,
    Vars& vars) {
  vars["method_name"] = method->name();
  vars["request_object"] = internal::ProtoName2CppName(method->input_type()->full_name());
  vars["response_object"] = internal::ProtoName2CppName(method->output_type()->full_name());
}

std::vector<std::string> BuildIncludes(pb::ServiceDescriptor const* service) {
  return std::vector<std::string>();
}

std::vector<std::string> BuildNamespaces(
    pb::ServiceDescriptor const* service) {
  return std::vector<std::string>();
}

void PrintMethods(
    pb::ServiceDescriptor const* service,
    Vars& vars,
    pb::io::Printer* p,
    char const* tmplt) {
  for (int i = 0; i < service->method_count(); i++) {
    const pb::MethodDescriptor* method = service->method(i);
    SetMethodVars(method, vars);
    p->Print(vars, tmplt);
  }
}

bool GenerateClientHeaderFile(
    pb::ServiceDescriptor const* service,
    Vars& vars,
    pb::io::Printer* p,
    std::string* error) {
  SetServiceVars(service, vars);
  auto includes = BuildIncludes(service);
  auto namespaces = BuildNamespaces(service);

  p->Print(vars, internal::kHeaderFileStartTemplate);
  for (auto include : includes) {
    p->Print(internal::kIncludeTemplate, "include", include);
  }
  for (auto nspace : namespaces) {
    p->Print(internal::kNamespaceStartTemplate, "namespace", nspace);
  }

  p->Print(vars, internal::kHeaderClientStartTemplate);
  PrintMethods(service, vars, p, internal::kHeaderClientMethodTemplate);

  p->Print(vars, internal::kHeaderClientEndTemplate);

  for (auto nspace : namespaces) {
    p->Print(internal::kNamespaceEndTemplate, "namespace", nspace);
  }
  return true;
}

bool GapicGenerator::Generate(pb::FileDescriptor const* file,
                                 std::string const& /* parameter */,
                                 pb::compiler::GeneratorContext* generator_context,
                                 std::string* error) const {
  if (file->options().cc_generic_services()) {
    *error =
        "cpp gapic proto compiler plugin does not work with generic "
        "services. To generate cpp grpc APIs, please set \""
        "cc_generic_service = false\".";
    return false;
  }

  Vars vars;

  for (int i = 0; i < file->service_count(); i++) {
    pb::ServiceDescriptor const* service = file->service(i);
    std::string service_header_name = internal::FileName(service->full_name());
    std::unique_ptr<pb::io::ZeroCopyOutputStream> gapic_service_output(
      generator_context->Open(service_header_name));
    pb::io::Printer printer(gapic_service_output.get(), '$', NULL);

    if (!GenerateClientHeaderFile(service, vars, &printer, error)) {
      return false;
    }
  }
  return true;
}

} // namespace codegen
} // namespace api
} // namespace google

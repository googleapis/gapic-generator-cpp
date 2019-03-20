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
#ifndef GOOGLE_API_CODEGEN_INTERNAL_DATA_MODEL_H_
#define GOOGLE_API_CODEGEN_INTERNAL_DATA_MODEL_H_

#include <algorithm>
#include <string>
#include <cctype>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/str_replace.h"
#include "src/google/protobuf/compiler/code_generator.h"
#include "src/google/protobuf/descriptor.h"
#include "src/google/protobuf/descriptor.pb.h"
#include "google/api/client.pb.h"

#include "gapic_utils.h"
#include "printer.h"

namespace pb = google::protobuf;

namespace google {
namespace api {
namespace codegen {
namespace internal {

struct DataModel {
  static void SetServiceVars(
      pb::ServiceDescriptor const* service,
      std::map<std::string, std::string>& vars) {
    vars["class_name"] = service->name();
    vars["proto_file_name"] = service->file()->name();
    vars["header_include_guard_const"] = absl::StrCat(service->name(), "_H_");
    vars["class_comment_block"] = "// TODO: pull in comments";
    vars["grpc_stub_fqn"] = internal::ProtoNameToCppName(service->full_name());
    vars["service_endpoint"] = service->options().GetExtension(google::api::default_host);
  }

  static void SetMethodVars(
      pb::MethodDescriptor const* method,
      std::map<std::string, std::string>& vars) {
    vars["method_name"] = method->name();
    vars["request_object"] = internal::ProtoNameToCppName(method->input_type()->full_name());
    vars["response_object"] = internal::ProtoNameToCppName(method->output_type()->full_name());
  }

  static void PrintMethods(
      pb::ServiceDescriptor const* service,
      std::map<std::string, std::string> vars,
      Printer& p,
      char const* tmplt) {
    for (int i = 0; i < service->method_count(); i++) {
      const pb::MethodDescriptor* method = service->method(i);
      SetMethodVars(method, vars);
      p->Print(vars, tmplt);
    }
  }
};

} // internal
} // codegen
} // api
} // google

#endif // GOOGLE_API_CODEGEN_INTERNAL_DATA_MODEL_H_

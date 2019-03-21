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
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "generator/gapic_generator.h"
#include "generator/internal/client_cc_generator.h"
#include "generator/internal/client_header_generator.h"
#include "generator/internal/data_model.h"
#include "generator/internal/gapic_utils.h"
#include "generator/internal/printer.h"

#include "google/api/client.pb.h"

namespace pb = google::protobuf;

namespace google {
namespace api {
namespace codegen {

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

  for (int i = 0; i < file->service_count(); i++) {
    pb::ServiceDescriptor const* service = file->service(i);

    // TODO(michaelbausor): initialize Vars with cross-file-descriptor
    // configuration, e.g. metadata annotation.
    std::map<std::string, std::string> vars;
    internal::DataModel::SetServiceVars(service, vars);

    std::string service_file_path =
        internal::ServiceNameToFilePath(service->full_name());

    std::string header_file_path = absl::StrCat(service_file_path, ".gapic.h");
    internal::Printer header_printer(generator_context, header_file_path);
    if (!internal::GenerateClientHeader(service, vars, header_printer, error)) {
      return false;
    }

    std::string header_stub_file_path =
        absl::StrCat(service_file_path, "_stub", ".gapic.h");
    internal::Printer header_stub_printer(generator_context,
                                          header_stub_file_path);
    if (!internal::GenerateClientStubHeader(service, vars, header_stub_printer,
                                            error)) {
      return false;
    }

    std::string cc_file_path = absl::StrCat(service_file_path, ".gapic.cc");
    internal::Printer cc_printer(generator_context, cc_file_path);
    if (!internal::GenerateClientCC(service, vars, cc_printer, error)) {
      return false;
    }

    std::string cc_stub_file_path =
        absl::StrCat(service_file_path, "_stub", ".gapic.cc");
    internal::Printer cc_stub_printer(generator_context, cc_stub_file_path);
    if (!internal::GenerateClientStubCC(service, vars, cc_stub_printer,
                                        error)) {
      return false;
    }
  }
  return true;
}

}  // namespace codegen
}  // namespace api
}  // namespace google

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
#ifndef GOOGLE_API_CODEGEN_INTERNAL_GAPIC_UTILS_H_
#define GOOGLE_API_CODEGEN_INTERNAL_GAPIC_UTILS_H_

#include <algorithm>
#include <string>
#include <cctype>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/str_replace.h"

namespace google {
namespace api {
namespace codegen {
namespace internal {

/**
 * Convert a CamelCase string to snake_case.
 */
inline std::string CamelCaseToSnakeCase(std::string const& input) {
    std::string output;
    for (auto i = 0u; i < input.size(); ++i) {
        if (i + 2 < input.size()) {
            if (std::isupper(input[i + 1]) && std::islower(input[i + 2])) {
                absl::StrAppend(&output, std::tolower(input[i]), "_");
                continue;
            }
        }
        if ((std::islower(input[i]) || std::isdigit(input[i]))
                && std::isupper(input[i + 1])) {
            absl::StrAppend(&output, std::tolower(input[i]), "_");
            continue;
        }
        absl::StrAppend(&output, std::tolower(input[i]));
    }
    return output;
}

/**
 * Convert a service name to a file path.
 *
 * service_name should consist of CamelCase pieces and "." separators.
 * Each component of service_name will become part of the path, except
 * the last component, which will become the file name. Components will
 * be converted from CamelCase to snake_case.
 *
 * Example: "google.LibraryService" -> "google/library_service"
 */
inline std::string ServiceNameToFilePath(std::string const& service_name) {
  std::vector<std::string> components = absl::StrSplit(service_name, '.');
  std::transform(components.begin(), components.end(), components.begin(),
                 CamelCaseToSnakeCase);
  return absl::StrJoin(components, "/");
}

/**
 * Convert a protobuf name to a fully qualified C++ name.
 *
 * proto_name should be a "." separated name, which we convert to a
 * "::" separated C++ fully qualified name.
 */
inline std::string ProtoNameToCppName(std::string const& proto_name) {
  return "::" + absl::StrReplaceAll(proto_name, {
    {".", "::"}});
}

} // internal
} // codegen
} // api
} // google

#endif // GOOGLE_API_CODEGEN_INTERNAL_GAPIC_UTILS_H_

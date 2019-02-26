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

inline std::string CamelCaseToSnakeCase(const std::string& input) {
  std::string output;
  // Initialize prev_char_is_upper to true so that we omit a leading "_"
  bool prev_char_is_upper = true;
  for (unsigned int i = 0; i < input.size(); i++) {
    char c = input[i];
    if (std::isupper(c) && !prev_char_is_upper) {
      absl::StrAppend(&output, "_", std::string(1, std::tolower(c)));
    } else {
      absl::StrAppend(&output, std::string(1, std::tolower(c)));
    }
    prev_char_is_upper = std::isupper(c);
  }
  return output;
}

inline std::string FileName(const std::string& service_name) {
  std::vector<std::string> components = absl::StrSplit(service_name, '.');
  std::transform(components.begin(), components.end(), components.begin(),
                 CamelCaseToSnakeCase);
  return absl::StrCat(absl::StrJoin(components, "/"), ".gapic.h");
}

inline std::string ProtoName2CppName(const std::string &proto_name) {
  return "::" + absl::StrReplaceAll(proto_name, {
    {".", "::"}});
}

} // internal
} // codegen
} // api
} // google

#endif // GOOGLE_API_CODEGEN_INTERNAL_GAPIC_UTILS_H_

// Copyright 2018 Google Inc.  All rights reserved
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

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

namespace google {
namespace api {
namespace codegen {
namespace internal {

inline std::string CamelCaseToSnakeCase(const std::string& input) {
  std::string output = "";
  std::string join = "";
  for (unsigned int i = 0; i < input.size(); i++) {
    char c = input[i];
    if (c >= 'A' && c <= 'Z') {
      absl::StrAppend(&output, join, std::string(1, c + 'a' - 'A'));
    } else {
      absl::StrAppend(&output, std::string(1, c));
    }
    join = "_";
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

std::string GetFileNameWithoutExtension(const std::string &full_file_name) {
  std::vector<absl::string_view> components = absl::StrSplit(full_file_name, '/');
  std::pair<std::string, std::string> p = absl::StrSplit(components.back(), '.');
  return p.first;
}

} // internal
} // codegen
} // api
} // google

#endif // GOOGLE_API_CODEGEN_INTERNAL_GAPIC_UTILS_H_

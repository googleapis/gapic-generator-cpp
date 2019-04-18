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
#ifndef GAPIC_GENERATOR_CPP_GENERATOR_INTERNAL_GAPIC_UTILS_H_
#define GAPIC_GENERATOR_CPP_GENERATOR_INTERNAL_GAPIC_UTILS_H_

#include <algorithm>
#include <cctype>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"

#include <google/protobuf/descriptor.h>

namespace google {
namespace api {
namespace codegen {
namespace internal {

namespace pb = google::protobuf;

bool NoStreamingPredicate(pb::MethodDescriptor const* m);

// Convenience functions for wrapping include headers with the correct
// delimiting characters (either <> or "")
std::string LocalInclude(std::string header);
std::string SystemInclude(std::string header);

/**
 * Convert a CamelCase string to snake_case.
 */
std::string CamelCaseToSnakeCase(std::string const& input);

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
std::string ServiceNameToFilePath(std::string const& service_name);

/**
 * Convert a protobuf name to a fully qualified C++ name.
 *
 * proto_name should be a "." separated name, which we convert to a
 * "::" separated C++ fully qualified name.
 */
std::string ProtoNameToCppName(std::string const& proto_name);

}  // namespace internal
}  // namespace codegen
}  // namespace api
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GENERATOR_INTERNAL_GAPIC_UTILS_H_

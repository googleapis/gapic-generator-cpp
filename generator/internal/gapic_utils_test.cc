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

#include "generator/internal/gapic_utils.h"
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

namespace google {
namespace api {
namespace codegen {
namespace internal {
namespace {

TEST(GapicUtils, CamelCaseToSnakeCase) {
  std::vector<std::pair<std::string, std::string>> test_cases{
      {"abc", "abc"},        {"Abc", "abc"},
      {"AbcDef", "abc_def"}, {"abcDef", "abc_def"},
      {"ABc", "a_bc"},       {"ABcDEf", "a_bc_d_ef"},
      {"Abc1De", "abc1_de"}, {"Abc1de", "abc1de"},
      {"A1B", "a1_b"},       {"SNMPParse", "snmp_parse"}};

  for (auto test_case : test_cases) {
    std::string actual = CamelCaseToSnakeCase(test_case.first);
    std::string expected = test_case.second;
    EXPECT_EQ(expected, actual);
  }
}

}  // namespace
}  // namespace internal
}  // namespace codegen
}  // namespace api
}  // namespace google

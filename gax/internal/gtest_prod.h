// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GAPIC_GENERATOR_CPP_GAX_INTERNAL_GTEST_PROD_H_
#define GAPIC_GENERATOR_CPP_GAX_INTERNAL_GTEST_PROD_H_

// Note: gtest-1.8.1 doesn't have a separate target for gtest_prod.
//       Rather than add an overlarge dependency on @gtest//:gtest,
//       just define the necessary macro ourselves.
#define FRIEND_TEST(test_case_name, test_name) \
  friend class test_case_name##_##test_name##_Test

#endif  // GAPIC_GENERATOR_CPP_GAX_INTERNAL_GTEST_PROD_H_

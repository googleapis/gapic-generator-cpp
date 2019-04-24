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

#ifndef GAPIC_GENERATOR_CPP_GAX_INTERNAL_TEST_CLOCK_H_
#define GAPIC_GENERATOR_CPP_GAX_INTERNAL_TEST_CLOCK_H_

#include <chrono>

namespace google {
namespace gax {
namespace internal {

// Each test that uses TestClock defines its own static now_point in the
// google::gax::internal namespace.
// The 'now' method and now_point must be static because
// std::chrono::system_clock::now is static. The alternative would be to store a
// clock instance in whatever types need a clock, which is unnecessarily
// heavyweight just to support testing.
extern std::chrono::time_point<std::chrono::system_clock> now_point;

class TestClock {
 public:
  static inline std::chrono::time_point<std::chrono::system_clock> now() {
    return now_point;
  }

  static std::chrono::time_point<std::chrono::system_clock> now_point;
};

}  // namespace internal
}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_INTERNAL_TEST_CLOCK_H_

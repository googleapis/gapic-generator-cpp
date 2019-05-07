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

/*
 * A clock with an external user-mutable now point.
 *
 * Unit tests should be deterministic, but some features and associated tests
 * rely on clocks. The solution is to use a TestClock as an injected parameter,
 * with an automatic time_point passed into the constructor.
 *
 * E.g.:
 *
 * std::chrono::system_clock::time_point n;
 * ClockUser<TestClock> cu(TestClock(n));
 * n += std::chrono::milliseconds(20);
 * cu.CheckElapsedTime();
 */
class TestClock {
 public:
  TestClock(std::chrono::system_clock::time_point& now_point)
      : now_point_(now_point) {}
  std::chrono::system_clock::time_point now() const { return now_point_; }

 private:
  std::chrono::system_clock::time_point& now_point_;
};

}  // namespace internal
}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_INTERNAL_TEST_CLOCK_H_

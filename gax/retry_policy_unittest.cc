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

#include "googletest/include/gtest/gtest.h"
#include "retry_policy.h"
#include <chrono>
#include <memory>

namespace {
using namespace ::google;

class TestStatus {
 public:
  bool const isPermanent;
};

class TestRetryablePolicy {
 public:
  static bool IsPermanentFailure(TestStatus const& s) {
    return s.isPermanent;
  }
};

using RP = gax::RetryPolicy<TestStatus, TestRetryablePolicy>;

using LECRP = gax::LimitedErrorCountRetryPolicy<TestStatus, TestRetryablePolicy>;

TEST(LimitedErrorCountRetryPolicy, Basic) {
  LECRP tested(3);
  TestStatus s{false};
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, PermanentFailureCheck) {
  LECRP tested(3);
  TestStatus s{true};
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, CopyConstruct) {
  LECRP tested(3);
  TestStatus s{false};
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  LECRP copy(tested);
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_FALSE(copy.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, MoveConstruct) {
  LECRP tested(3);
  TestStatus s{false};
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  LECRP copy(std::move(tested));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_FALSE(copy.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, Clone) {
  LECRP tested(3);
  TestStatus s{false};
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  std::unique_ptr<RP> clone = tested.clone();
  EXPECT_TRUE(clone->OnFailure(s));
  EXPECT_TRUE(clone->OnFailure(s));
  EXPECT_TRUE(clone->OnFailure(s));
  EXPECT_FALSE(clone->OnFailure(s));
}

static std::chrono::time_point<std::chrono::system_clock> now_point = std::chrono::system_clock::now();

class TestClock {
 public:
  static inline std::chrono::time_point<std::chrono::system_clock> now() {
    return now_point;
  }
};

using LDRP = gax::LimitedDurationRetryPolicy<TestStatus, TestRetryablePolicy, TestClock>;

TEST(LimitedDurationRetryPolicy, Basic) {
  LDRP tested(std::chrono::milliseconds(5));
  TestStatus s{false};
  EXPECT_TRUE(tested.OnFailure(s));

  now_point += std::chrono::milliseconds(2);
  EXPECT_TRUE(tested.OnFailure(s));

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, PermanentFailureCheck) {
  LDRP tested(std::chrono::milliseconds(5));
  TestStatus s{true};

  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, CopyConstruct) {
  LDRP tested(std::chrono::milliseconds(5));
  TestStatus s{false};

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  LDRP copy(tested);
  EXPECT_TRUE(copy.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, MoveConstruct) {
  LDRP tested(std::chrono::milliseconds(5));
  TestStatus s{false};

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  LDRP copy(std::move(tested));
  EXPECT_TRUE(copy.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, Clone) {
  LDRP tested(std::chrono::milliseconds(5));
  TestStatus s{false};

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  std::unique_ptr<RP> clone = tested.clone();
  EXPECT_TRUE(clone->OnFailure(s));
}

}  // namespace

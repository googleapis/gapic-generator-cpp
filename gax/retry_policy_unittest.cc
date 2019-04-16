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

#include <chrono>
#include <memory>

#include <gtest/gtest.h>
#include "retry_policy.h"
#include "status.h"

namespace {
using namespace ::google;

TEST(LimitedErrorCountRetryPolicy, Basic) {
  gax::LimitedErrorCountRetryPolicy tested(3);
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, PermanentFailureCheck) {
  gax::LimitedErrorCountRetryPolicy tested(3);
  gax::Status s{gax::StatusCode::kCancelled, ""};
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, CopyConstruct) {
  gax::LimitedErrorCountRetryPolicy tested(3);
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedErrorCountRetryPolicy copy(tested);
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_FALSE(copy.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, MoveConstruct) {
  gax::LimitedErrorCountRetryPolicy tested(3);
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedErrorCountRetryPolicy copy(std::move(tested));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_FALSE(copy.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, Clone) {
  gax::LimitedErrorCountRetryPolicy tested(3);
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  std::unique_ptr<gax::RetryPolicy> clone = tested.clone();
  EXPECT_TRUE(clone->OnFailure(s));
  EXPECT_TRUE(clone->OnFailure(s));
  EXPECT_TRUE(clone->OnFailure(s));
  EXPECT_FALSE(clone->OnFailure(s));
}

static std::chrono::time_point<std::chrono::system_clock> now_point;

class TestClock {
 public:
  static inline std::chrono::time_point<std::chrono::system_clock> now() {
    return now_point;
  }
};

TEST(LimitedDurationRetryPolicy, Basic) {
  gax::LimitedDurationRetryPolicy<TestClock> tested(
      std::chrono::milliseconds(5));
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));

  now_point += std::chrono::milliseconds(2);
  EXPECT_TRUE(tested.OnFailure(s));

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, PermanentFailureCheck) {
  gax::LimitedDurationRetryPolicy<TestClock> tested(
      std::chrono::milliseconds(5));
  gax::Status s{gax::StatusCode::kCancelled, ""};

  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, CopyConstruct) {
  gax::LimitedDurationRetryPolicy<TestClock> tested(
      std::chrono::milliseconds(5));
  gax::Status s;

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedDurationRetryPolicy<TestClock> copy(tested);
  EXPECT_TRUE(copy.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, MoveConstruct) {
  gax::LimitedDurationRetryPolicy<TestClock> tested(
      std::chrono::milliseconds(5));
  gax::Status s;

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedDurationRetryPolicy<TestClock> copy(std::move(tested));
  EXPECT_TRUE(copy.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, Clone) {
  gax::LimitedDurationRetryPolicy<TestClock> tested(
      std::chrono::milliseconds(5));
  gax::Status s;

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  std::unique_ptr<gax::RetryPolicy> clone = tested.clone();
  EXPECT_TRUE(clone->OnFailure(s));
}

}  // namespace

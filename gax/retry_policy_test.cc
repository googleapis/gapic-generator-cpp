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

#include "gax/retry_policy.h"
#include "gax/internal/test_clock.h"
#include "gax/status.h"
#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <memory>

namespace {
using namespace ::google;

TEST(LimitedErrorCountRetryPolicy, Basic) {
  gax::LimitedErrorCountRetryPolicy<> tested(3, std::chrono::milliseconds(30));
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, PermanentFailureCheck) {
  gax::LimitedErrorCountRetryPolicy<> tested(3, std::chrono::milliseconds(30));
  gax::Status s{gax::StatusCode::kCancelled, ""};
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, CopyConstruct) {
  gax::LimitedErrorCountRetryPolicy<> tested(3, std::chrono::milliseconds(30));
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedErrorCountRetryPolicy<> copy(tested);
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_FALSE(copy.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, MoveConstruct) {
  gax::LimitedErrorCountRetryPolicy<> tested(3, std::chrono::milliseconds(30));
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_TRUE(tested.OnFailure(s));
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedErrorCountRetryPolicy<> copy(std::move(tested));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_TRUE(copy.OnFailure(s));
  EXPECT_FALSE(copy.OnFailure(s));
}

TEST(LimitedErrorCountRetryPolicy, Clone) {
  gax::LimitedErrorCountRetryPolicy<> tested(3, std::chrono::milliseconds(30));
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

TEST(LimitedErrorCountRetryPolicy, OperationDeadline) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedErrorCountRetryPolicy<gax::internal::TestClock> tested(
      3, std::chrono::milliseconds(30), gax::internal::TestClock(now_point));

  EXPECT_EQ(tested.OperationDeadline(),
            now_point + std::chrono::milliseconds(30));

  auto clone = tested.clone();
  now_point += std::chrono::milliseconds(50);
  EXPECT_EQ(tested.OperationDeadline(), clone->OperationDeadline());
}

TEST(LimitedDurationRetryPolicy, Basic) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> tested(
      std::chrono::milliseconds(5), std::chrono::milliseconds(30),
      gax::internal::TestClock(now_point));
  gax::Status s;
  EXPECT_TRUE(tested.OnFailure(s));

  now_point += std::chrono::milliseconds(2);
  EXPECT_TRUE(tested.OnFailure(s));

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, PermanentFailureCheck) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> tested(
      std::chrono::milliseconds(5), std::chrono::milliseconds(30),
      gax::internal::TestClock(now_point));
  gax::Status s{gax::StatusCode::kCancelled, ""};

  EXPECT_FALSE(tested.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, CopyConstruct) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> tested(
      std::chrono::milliseconds(5), std::chrono::milliseconds(30),
      gax::internal::TestClock(now_point));
  gax::Status s;

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> copy(tested);
  EXPECT_TRUE(copy.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, MoveConstruct) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> tested(
      std::chrono::milliseconds(5), std::chrono::milliseconds(30),
      gax::internal::TestClock(now_point));
  gax::Status s;

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> copy(
      std::move(tested));
  EXPECT_TRUE(copy.OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, Clone) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> tested(
      std::chrono::milliseconds(5), std::chrono::milliseconds(30),
      gax::internal::TestClock(now_point));
  gax::Status s;

  now_point += std::chrono::milliseconds(10);
  EXPECT_FALSE(tested.OnFailure(s));

  std::unique_ptr<gax::RetryPolicy> clone = tested.clone();
  EXPECT_TRUE(clone->OnFailure(s));
}

TEST(LimitedDurationRetryPolicy, OperationDeadline) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> tested(
      std::chrono::milliseconds(500), std::chrono::milliseconds(30),
      gax::internal::TestClock(now_point));

  EXPECT_EQ(tested.OperationDeadline(),
            now_point + std::chrono::milliseconds(30));

  auto clone = tested.clone();
  now_point += std::chrono::milliseconds(50);
  EXPECT_EQ(tested.OperationDeadline(), clone->OperationDeadline());
}

TEST(LimitedDurationRetryPolicy, OperationDeadlineCap) {
  std::chrono::system_clock::time_point now_point;
  gax::LimitedDurationRetryPolicy<gax::internal::TestClock> tested(
      std::chrono::milliseconds(500), std::chrono::milliseconds(30),
      gax::internal::TestClock(now_point));

  // Don't exceed the overarching timeout
  now_point += std::chrono::milliseconds(490);
  EXPECT_EQ(tested.OperationDeadline(),
            now_point + std::chrono::milliseconds(10));
}

}  // namespace

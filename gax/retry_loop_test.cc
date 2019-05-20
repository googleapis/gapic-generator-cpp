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

#include "gax/retry_loop.h"
#include "google/longrunning/operations.pb.h"
#include "gax/backoff_policy.h"
#include "gax/call_context.h"
#include "gax/internal/test_clock.h"
#include "gax/retry_policy.h"
#include <gtest/gtest.h>
#include <chrono>

namespace {
using namespace ::google;

class DummyBackoffPolicy : public gax::BackoffPolicy {
 public:
  // Hack to count the number of backoff attempts.
  int& delay_count_;
  DummyBackoffPolicy(int& delay_count) : delay_count_(delay_count) {}

  std::chrono::microseconds OnCompletion() override {
    delay_count_++;
    return std::chrono::microseconds(0);
  }
  std::unique_ptr<gax::BackoffPolicy> clone() const override {
    return std::unique_ptr<DummyBackoffPolicy>(
        new DummyBackoffPolicy(delay_count_));
  }
};

std::unique_ptr<gax::BackoffPolicy> DummyBackoffFactory(int& delay_count) {
  return std::unique_ptr<DummyBackoffPolicy>(
      new DummyBackoffPolicy(delay_count));
}

std::unique_ptr<gax::RetryPolicy> ErrCountRetryFactory(
    int n, std::chrono::system_clock::time_point& now_point) {
  return std::unique_ptr<
      gax::LimitedErrorCountRetryPolicy<gax::internal::TestClock>>(
      new gax::LimitedErrorCountRetryPolicy<gax::internal::TestClock>(
          n, std::chrono::milliseconds(2), now_point));
}

TEST(RetryLoop, Basic) {
  gax::MethodInfo mi{"TestMethod", gax::MethodInfo::RpcType::CLIENT_STREAMING,
                     gax::MethodInfo::Idempotency::IDEMPOTENT};
  gax::CallContext context(mi);
  longrunning::GetOperationRequest req;
  longrunning::Operation resp;
  std::chrono::system_clock::time_point now_point;

  int attempts_remaining = 3;
  auto fail_until = [&attempts_remaining, &context](
      gax::CallContext& ctx, longrunning::GetOperationRequest const& req,
      longrunning::Operation* resp) {
    // Make sure each retry has a fresh context.
    EXPECT_NE(&context, &ctx);
    return ((attempts_remaining--) > 1)
               ? gax::Status(gax::StatusCode::kAborted, "Aborted")
               : gax::Status{};
  };

  int delay_count = 0;
  gax::Status succeed = gax::MakeRetryCall<longrunning::GetOperationRequest,
                                           longrunning::Operation>(
      context, req, &resp, fail_until, ErrCountRetryFactory(10, now_point),
      DummyBackoffFactory(delay_count));
  EXPECT_EQ(attempts_remaining, 0);
  EXPECT_EQ(succeed, gax::Status());
  EXPECT_EQ(delay_count, 2);

  delay_count = 0;
  attempts_remaining = 10;
  gax::Status retry_timeout =
      gax::MakeRetryCall<longrunning::GetOperationRequest,
                         longrunning::Operation>(
          context, req, &resp, fail_until, ErrCountRetryFactory(3, now_point),
          DummyBackoffFactory(delay_count));
  EXPECT_EQ(retry_timeout, gax::Status(gax::StatusCode::kAborted, "Aborted"));
  EXPECT_EQ(attempts_remaining, 6);
  EXPECT_EQ(delay_count, 3);
}

TEST(RetryLoop, OperationDeadline) {
  gax::MethodInfo mi{"TestMethod", gax::MethodInfo::RpcType::CLIENT_STREAMING,
                     gax::MethodInfo::Idempotency::IDEMPOTENT};
  gax::CallContext context(mi);
  longrunning::GetOperationRequest req;
  longrunning::Operation resp;
  int delay_count = 0;
  std::chrono::system_clock::time_point now_point;

  auto check_updated_deadline = [&now_point](
      gax::CallContext& ctx, longrunning::GetOperationRequest const& req,
      longrunning::Operation* resp) {
    grpc::ClientContext test_context;
    ctx.PrepareGrpcContext(&test_context);

    EXPECT_EQ(test_context.deadline(),
              now_point + std::chrono::milliseconds(2));
    // Double check that we're not setting the deadline from a static
    // target.
    now_point += std::chrono::milliseconds(30);
    // Need to fail in a retryable manner promote retry.
    return gax::Status(gax::StatusCode::kAborted, "Aborted");
  };

  gax::MakeRetryCall<longrunning::GetOperationRequest, longrunning::Operation>(
      context, req, &resp, check_updated_deadline,
      ErrCountRetryFactory(3, now_point), DummyBackoffFactory(delay_count));
}

}  // namespace

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

#include "gax/retry_loop.h"
#include "google/longrunning/operations.pb.h"
#include "googletest/include/gtest/gtest.h"
#include "gax/backoff_policy.h"
#include "gax/call_context.h"
#include "gax/retry_policy.h"
#include <chrono>

namespace {
using namespace ::google;

std::unique_ptr<gax::RetryPolicy> ErrCountRetryFactory(int n) {
  return std::unique_ptr<gax::LimitedErrorCountRetryPolicy>(
      new gax::LimitedErrorCountRetryPolicy(n));
}

TEST(RetryLoop, Basic) {
  // Hack to count the number of backoff attempts.
  static int delay_count = 0;

  class DummyBackoffPolicy : public gax::BackoffPolicy {
   public:
    std::chrono::microseconds OnCompletion() override {
      delay_count++;
      return std::chrono::microseconds(0);
    }
    std::unique_ptr<gax::BackoffPolicy> clone() const override {
      return std::unique_ptr<DummyBackoffPolicy>(new DummyBackoffPolicy());
    }

    static std::unique_ptr<gax::BackoffPolicy> MakeDummyBackoff() {
      return std::unique_ptr<DummyBackoffPolicy>(new DummyBackoffPolicy());
    }
  };

  gax::MethodInfo mi{"TestMethod", gax::MethodInfo::RpcType::CLIENT_STREAMING,
                     gax::MethodInfo::Idempotency::IDEMPOTENT};
  gax::CallContext context(mi);
  longrunning::GetOperationRequest req;
  longrunning::Operation resp;

  int attempts_remaining = 3;
  auto fail_until = [&attempts_remaining, &context](
      gax::CallContext& ctx, longrunning::GetOperationRequest const& req,
      longrunning::Operation* resp) -> gax::Status {
    // Make sure each retry has a fresh context.
    EXPECT_NE(&context, &ctx);
    return ((attempts_remaining--) > 1)
               ? gax::Status(gax::StatusCode::kAborted, "Aborted")
               : gax::Status{};
  };

  gax::Status succeed = gax::MakeRetryCall<longrunning::GetOperationRequest,
                                           longrunning::Operation>(
      context, req, &resp, fail_until, ErrCountRetryFactory(10),
      DummyBackoffPolicy::MakeDummyBackoff());
  EXPECT_EQ(attempts_remaining, 0);
  EXPECT_EQ(succeed, gax::Status());
  EXPECT_EQ(delay_count, 2);

  delay_count = 0;
  attempts_remaining = 10;
  gax::Status retry_timeout =
      gax::MakeRetryCall<longrunning::GetOperationRequest,
                         longrunning::Operation>(
          context, req, &resp, fail_until, ErrCountRetryFactory(3),
          DummyBackoffPolicy::MakeDummyBackoff());
  EXPECT_EQ(retry_timeout, gax::Status(gax::StatusCode::kAborted, "Aborted"));
  EXPECT_EQ(attempts_remaining, 6);
  EXPECT_EQ(delay_count, 3);
}
}

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

#include "call_context.h"
#include "backoff_policy.h"
#include "googletest/include/gtest/gtest.h"
#include "retry_policy.h"

#include <chrono>
#include <set>
#include <string>
#include <type_traits>

namespace google {
namespace gax {

static_assert(std::is_move_constructible<CallContext>::value,
              "CallContext must be move constructable");

static_assert(std::is_copy_constructible<CallContext>::value,
              "CallContext must be copy constructable");

TEST(CallContext, Basic) {
  gax::MethodInfo mi{"TestMethod", MethodInfo::RpcType::CLIENT_STREAMING,
                     MethodInfo::Idempotency::IDEMPOTENT};
  gax::CallContext ctx(mi);
  EXPECT_EQ(ctx.Deadline(), std::chrono::system_clock::time_point::max());

  auto info = ctx.Info();
  EXPECT_EQ(std::string(info.rpc_name), "TestMethod");
  EXPECT_EQ(info.rpc_type, MethodInfo::RpcType::CLIENT_STREAMING);
  EXPECT_EQ(info.idempotency, MethodInfo::Idempotency::IDEMPOTENT);

  auto now = std::chrono::system_clock::now();
  ctx.SetDeadline(now);
  EXPECT_EQ(ctx.Deadline(), now);

  ctx.AddMetadata("testKey", "testVal");
  auto iter = ctx.metadata_.find("testKey");
  EXPECT_NE(iter, ctx.metadata_.end());
  EXPECT_EQ(iter->second, "testVal");

  std::set<std::string> const vals = {"testVal", "testVal2"};
  std::set<std::string> tmp;
  ctx.AddMetadata("testKey", "testVal2");
  EXPECT_EQ(ctx.metadata_.count("testKey"), std::size_t(2));
  auto range = ctx.metadata_.equal_range("testKey");
  for (auto i = range.first; i != range.second; ++i) {
    tmp.insert(i->second);
  }
  EXPECT_EQ(tmp, vals);

  int policy_invoked = 0;
  ctx.AddGrpcContextPolicy(
      [&policy_invoked](grpc::ClientContext*) { policy_invoked++; });

  grpc::ClientContext client_ctx;
  ctx.PrepareGrpcContext(&client_ctx);
  EXPECT_EQ(policy_invoked, 1);
  EXPECT_EQ(client_ctx.deadline(), now);

  // There isn't a good way to examine ClientContext metadata without it being
  // sent to a server, so take it on faith that it was properly added.
}

TEST(CallContext, CopyAndMove) {
  gax::MethodInfo mi{"TestMethod", MethodInfo::RpcType::CLIENT_STREAMING,
                     MethodInfo::Idempotency::IDEMPOTENT};
  gax::CallContext base(mi);

  gax::CallContext no_policy_copy(base);
  EXPECT_FALSE(no_policy_copy.RetryPolicy());
  EXPECT_FALSE(no_policy_copy.BackoffPolicy());
  gax::CallContext no_policy_move(std::move(no_policy_copy));
  EXPECT_FALSE(no_policy_move.RetryPolicy());
  EXPECT_FALSE(no_policy_move.BackoffPolicy());

  base.SetRetryPolicy(gax::LimitedErrorCountRetryPolicy(10));
  base.SetBackoffPolicy(gax::ExponentialBackoffPolicy(
      std::chrono::milliseconds(1), std::chrono::milliseconds(10)));
  gax::CallContext policy_copy(base);
  EXPECT_TRUE(policy_copy.RetryPolicy());
  EXPECT_TRUE(policy_copy.BackoffPolicy());
  gax::CallContext policy_move(std::move(base));
  EXPECT_TRUE(policy_move.RetryPolicy());
  EXPECT_TRUE(policy_move.BackoffPolicy());
}

}  // namespace gax
}  // namespace google

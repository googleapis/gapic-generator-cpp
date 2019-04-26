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

#include <chrono>

namespace google {
namespace gax {

void CallContext::SetDeadline(std::chrono::system_clock::time_point deadline) {
  deadline_ = std::move(deadline);
}

void CallContext::AddGrpcContextPolicy(GrpcContextPolicyFunc f) {
  context_policies_.emplace_back(std::move(f));
}

void CallContext::PrepareGrpcContext(grpc::ClientContext* context) {
  context->set_deadline(deadline_);

  for (auto const& m : metadata_) {
    context->AddMetadata(m.first, m.second);
  }

  for (auto const& f : context_policies_) {
    f(context);
  }
}

void CallContext::AddMetadata(std::string key, std::string val) {
  metadata_.emplace(std::move(key), std::move(val));
}

MethodInfo CallContext::Info() const { return method_info_; }

std::unique_ptr<gax::RetryPolicy> CallContext::RetryPolicy() const {
  return retry_policy_ ? retry_policy_->clone() : nullptr;
}

std::unique_ptr<gax::BackoffPolicy> CallContext::BackoffPolicy() const {
  return backoff_policy_ ? backoff_policy_->clone() : nullptr;
}

void CallContext::SetRetryPolicy(gax::RetryPolicy const& retry_policy) {
  retry_policy_ = retry_policy.clone();
}

void CallContext::SetBackoffPolicy(gax::BackoffPolicy const& backoff_policy) {
  backoff_policy_ = backoff_policy.clone();
}

std::chrono::system_clock::time_point CallContext::Deadline() const {
  return deadline_;
}
}
}

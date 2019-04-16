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

bool CallContext::AddMetadata(std::string key, std::string val) {
  return metadata_.emplace(std::move(key), std::move(val)).second;
}

std::chrono::system_clock::time_point CallContext::Deadline() const {
  return deadline_;
}
}
}

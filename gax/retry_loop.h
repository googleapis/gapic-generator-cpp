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

#ifndef GAPIC_GENERATOR_CPP_GAX_RETRY_LOOP_H_
#define GAPIC_GENERATOR_CPP_GAX_RETRY_LOOP_H_

#include "gax/backoff_policy.h"
#include "gax/call_context.h"
#include "gax/internal/invoke_result.h"
#include "gax/retry_policy.h"
#include "gax/status.h"
#include <functional>
#include <thread>

namespace google {
namespace gax {

template <typename RequestT, typename ResponseT, typename FunctorT,
          typename std::enable_if<
              gax::internal::is_invocable<FunctorT, gax::CallContext&,
                                          RequestT const&, ResponseT*>::value,
              int>::type = 0>
gax::Status MakeRetryCall(gax::CallContext& context, RequestT const& request,
                          ResponseT* response, FunctorT&& next_stub,
                          std::unique_ptr<gax::RetryPolicy> retry_policy,
                          std::unique_ptr<gax::BackoffPolicy> backoff_policy) {
  while (true) {
    // The next layer stub may add metadata, so create a
    // fresh call context each time through the loop.
    gax::CallContext context_copy(context);
    context_copy.SetDeadline(retry_policy->OperationDeadline());
    gax::Status status = next_stub(context_copy, request, response);
    if (status.IsOk() || !retry_policy->OnFailure(status)) {
      return status;
    }

    std::this_thread::sleep_for(backoff_policy->OnCompletion());
  }
}

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_RETRY_LOOP_H_

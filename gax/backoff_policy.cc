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

#include "gax/backoff_policy.h"
#include <algorithm>
#include <chrono>
#include <memory>
#include <random>

namespace google {
namespace gax {

std::chrono::microseconds ExponentialBackoffPolicy::OnCompletion() {
  if (!generator_) {
    generator_ = std::unique_ptr<std::mt19937_64>(
        new std::mt19937_64(std::random_device()()));
  }

  std::uniform_int_distribution<std::chrono::microseconds::rep> dist(
      current_delay_range_.count() / 2, current_delay_range_.count());
  auto delay = std::chrono::microseconds(dist(*generator_));

  current_delay_range_ = std::min(maximum_delay_, current_delay_range_ * 2);
  return delay;
}

std::unique_ptr<BackoffPolicy> ExponentialBackoffPolicy::clone() const {
  return std::unique_ptr<BackoffPolicy>(new ExponentialBackoffPolicy(*this));
}

}  // namespace gax
}  // namespace google

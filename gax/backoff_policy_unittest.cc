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
#include <random>

#include "googletest/include/gtest/gtest.h"

#include "backoff_policy.h"

namespace google {
namespace gax {

TEST(ExponentialBackoffPolicy, Basic) {
  ExponentialBackoffPolicy tested(std::chrono::milliseconds(1),
                                  std::chrono::milliseconds(32));

  for(int i = 0; i < 5; ++i){
    auto base_delay = tested.current_delay_range_;
    EXPECT_EQ(base_delay, std::chrono::milliseconds(1<<i));
    auto delay = tested.OnCompletion();
    EXPECT_GE(delay, base_delay/2.0);
    EXPECT_LE(delay, base_delay);
  }

  EXPECT_EQ(tested.current_delay_range_, tested.maximum_delay_);
  // current_delay_range_ saturates to max delay.
  tested.OnCompletion();
  EXPECT_EQ(tested.current_delay_range_, tested.maximum_delay_);
}

TEST(ExponentialBackoffPolicy, CopyConstruct) {
  ExponentialBackoffPolicy tested(std::chrono::milliseconds(10),
                                  std::chrono::milliseconds(320));
  tested.OnCompletion();
  ExponentialBackoffPolicy copy(tested);  // Copy starts with fresh backoff delays

  EXPECT_EQ(copy.current_delay_range_, std::chrono::milliseconds(10));
  EXPECT_EQ(copy.maximum_delay_, std::chrono::milliseconds(320));
}

TEST(ExponentialBackoffPolicy, MoveConstruct) {
  ExponentialBackoffPolicy tested(std::chrono::milliseconds(10),
                                  std::chrono::milliseconds(320));
  tested.OnCompletion();
  ExponentialBackoffPolicy moved(std::move(tested));  // Starts with fresh backoff delays

  EXPECT_EQ(moved.current_delay_range_, std::chrono::milliseconds(10));
  EXPECT_EQ(moved.maximum_delay_, std::chrono::milliseconds(320));
}

TEST(ExponentialBackoffPolicy, Clone) {
  ExponentialBackoffPolicy tested(std::chrono::milliseconds(10),
                                  std::chrono::milliseconds(320));
  tested.OnCompletion();
  std::unique_ptr<BackoffPolicy> clone = tested.clone();

  // We need to check that the clone method has the right signature, but we also need
  // to check that the clone attributes have the right initial values.
  auto cast_clone = std::unique_ptr<ExponentialBackoffPolicy>(
      static_cast<ExponentialBackoffPolicy*>(clone.release()));

  EXPECT_EQ(cast_clone->current_delay_range_, std::chrono::milliseconds(10));
  EXPECT_EQ(cast_clone->maximum_delay_, std::chrono::milliseconds(320));
}

TEST(ExponentialBackoffPolicy, LazyGenerator) {
  ExponentialBackoffPolicy tested(std::chrono::milliseconds(10),
                                  std::chrono::milliseconds(320));
  EXPECT_EQ(tested.generator_, nullptr);
  tested.OnCompletion();
  EXPECT_NE(tested.generator_, nullptr);

  // Copies and clones use their own lazily constructed generators
  ExponentialBackoffPolicy copy(tested);
  EXPECT_EQ(copy.generator_, nullptr);
  copy.OnCompletion();
  EXPECT_NE(copy.generator_, nullptr);

  auto clone = std::unique_ptr<ExponentialBackoffPolicy>(
      static_cast<ExponentialBackoffPolicy*>(copy.clone().release()));
  EXPECT_EQ(clone->generator_, nullptr);
  clone->OnCompletion();
  EXPECT_NE(clone->generator_, nullptr);

  // Moves reuse the existing generator
  ExponentialBackoffPolicy move(std::move(tested));
  EXPECT_NE(move.generator_, nullptr);
}

}  // namespace gax
}  // namespace google

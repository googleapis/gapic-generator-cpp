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

#ifndef GOOGLE_GAX_BACKOFF_POLICY_H_
#define GOOGLE_GAX_BACKOFF_POLICY_H_

#include <chrono>
#include <memory>
#include <random>

#include "internal/gtest_prod.h"

namespace google {
namespace gax {

/**
 * Define the interface for backoff policies.
 *
 * The client libraries need to hide partial and temporary failures from the
 * application. Exponential backoff is generally considered a best practice when
 * retrying operations. However, the details of how exponetial backoff is
 * implemented and tuned varies widely. We need to give the users enough
 * flexibility, and also provide sensible default implementations.
 *
 * The client library receives an object of this type and clones a new instance
 * for each operation. That is, the application provides the library with a
 * [Prototype](https://en.wikipedia.org/wiki/Prototype_pattern) of the policy
 * that will be applied to each operation.
 *
 * [Truncated Exponential
 * Backoff](https://cloud.google.com/storage/docs/exponential-backoff) in the
 * Google Cloud Storage documentation.
 *
 */
class BackoffPolicy {
 public:
  virtual ~BackoffPolicy() = default;

  /**
   * Handle an operation completion.
   *
   * This function is called when an operation has failed and needs to be retried.
   * The decision to retry or not is handled by other policies.
   *
   * @return the delay to wait before the next retry attempt.
   */
  virtual std::chrono::microseconds OnCompletion() = 0;

  /**
   * Return a new copy of this object.
   */
  virtual std::unique_ptr<BackoffPolicy> clone() const = 0;
};

/**
 * Implements a truncated exponential backoff with randomization policy.
 *
 * This policy implements the truncated exponential backoff policy for
 * retrying operations. After a request fails, and subject to a separate
 * retry policy, the client library will wait for an initial delay before
 * trying again. If the second attempt fails the delay time is increased by a factor
 * of 2. The delay time growth stops at a maximum delay wait time.
 * The policy also randomizes the delay each time, to avoid
 * [thundering herd
 * problem](https://en.wikipedia.org/wiki/Thundering_herd_problem).
 *
 * Note: The random number generator used when calculating backoff time in
 *       OnCompletion is lazily created. Furthermore, GeneratorFactory::Generator
 *       may touch mutable shared state.
 *       As a result, OnCompletion is NOT thread-safe.
 *       All other methods, including clone, are thread-safe.
 *       ExponentialBackoffPolicy is therefore thread-compatible.
 */
class ExponentialBackoffPolicy : BackoffPolicy {
 public:
  /**
   * Constructor for an exponential backoff policy.
   *
   * Define the initial delay and maximum delay for an instance
   * of the policy. While the constructor accepts `std::chrono::duration`
   * objects at any resolution, the data is kept internally in microseconds.
   * Sub-microsecond delays seem unnecessarily precise for this application.
   *
   * @code
   * using namespace std::chrono_literals; // C++14
   * auto r1 = ExponentialBackoffPolicy(10ms, 500ms);
   * @endcode
   *
   * @param initial_delay how long to wait after the first (unsuccessful)
   *     operation.
   * @param maximum_delay the maximum value for the delay between operations.
   *
   * @tparam duration1_t a placeholder to match the Rep tparam for @p initial_delay's
   *     type, the semantics of this template parameter are documented in
   *     `std::chrono::duration<>` (in brief, the underlying arithmetic type
   *     used to store the number of ticks), for our purposes it is simply a
   *     formal parameter.
   * @tparam d1 a placeholder to match the Period tparam for
   *     @p initial_delay's type, the semantics of this template parameter are
   *     documented in `std::chrono::duration<>` (in brief, the length of the
   *     tick in seconds, expressed as a `std::ratio<>`), for our purposes it
   *     is simply a formal parameter.
   * @tparam duration2_t similar formal parameter for the type of @p maximum_delay.
   * @tparam d2 similar formal parameter for the type of @p maximum_delay.
   *
   * @see
   * [std::chrono::duration<>](http://en.cppreference.com/w/cpp/chrono/duration)
   *     for more details.
   */
  template<typename duration1_t, typename duration2_t>
  ExponentialBackoffPolicy(duration1_t d1, duration2_t d2) :
      initial_delay_(std::chrono::duration_cast<std::chrono::microseconds>(d1)),
      current_delay_range_(initial_delay_),
      maximum_delay_(std::chrono::duration_cast<std::chrono::microseconds>(d2)) {}

  ExponentialBackoffPolicy(ExponentialBackoffPolicy const& rhs) noexcept
      : ExponentialBackoffPolicy(rhs.initial_delay_, rhs.maximum_delay_) {}

  ExponentialBackoffPolicy(ExponentialBackoffPolicy&& rhs) noexcept
      : initial_delay_(std::move(rhs.initial_delay_)),
        current_delay_range_(initial_delay_),
        maximum_delay_(std::move(rhs.maximum_delay_)),
        generator_(std::move(rhs.generator_)) {}

  std::chrono::microseconds OnCompletion() override;

  std::unique_ptr<BackoffPolicy> clone() const override;

 private:
  FRIEND_TEST(ExponentialBackoffPolicy, Basic);
  FRIEND_TEST(ExponentialBackoffPolicy, CopyConstruct);
  FRIEND_TEST(ExponentialBackoffPolicy, MoveConstruct);
  FRIEND_TEST(ExponentialBackoffPolicy, Clone);
  FRIEND_TEST(ExponentialBackoffPolicy, LazyGenerator);

  std::chrono::microseconds const initial_delay_;
  std::chrono::microseconds current_delay_range_;
  std::chrono::microseconds const maximum_delay_;

  // Store via pointer and do not initialize until OnCompletion is called.
  // The 19937 refers to bits of state: as a result, generator_ is very large,
  // is expensive to create, and in any case most rpcs succeed on the first call.
  std::unique_ptr<std::mt19937_64> generator_;
};

}  // namespace gax
}  // namespace google

#endif  // GOOGLE_GAX_BACKOFF_POLICY_H_

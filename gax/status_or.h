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

#ifndef GOOGLE_GAX_STATUS_OR_H_
#define GOOGLE_GAX_STATUS_OR_H_

#include <cstdlib>
#include <iostream>
#include <memory>

#include "status.h"

namespace google {
namespace gax {

/**
 * Holds a value or a `Status` indicating why there is no value.
 *
 * `StatusOr<T>` represents either a usable `T` value or a `Status` object
 * explaining why a `T` value is not present. Typical usage of `StatusOr<T>`
 * looks like usage of a smart pointer, or even a std::optional<T>, in that you
 * first check its validity using a conversion to bool (or by calling
 * `StatusOr::ok()`), then you may dereference the object to access the
 * contained value. It is undefined behavior (UB) to dereference a
 * `StatusOr<T>` that is not "ok". For example:
 *
 * @code
 * StatusOr<Foo> foo = FetchFoo();
 * if (!foo) {  // Same as !foo.ok()
 *   // handle error and probably look at foo.status()
 * } else {
 *   foo->DoSomethingFooey();  // UB if !foo
 * }
 * @endcode
 *
 * Alternatively, you may call the `StatusOr::value()` member function,
 * which will invoke `std::abort()` if `!StatusOr::ok()`.
 *
 * @code
 * StatusOr<Foo> foo = FetchFoo();
 * foo.value().DoSomethingFooey();  // May throw/crash if there is no value
 * @endcode
 *
 * Functions that can fail will often return a `StatusOr<T>` instead of
 * returning an error code and taking a `T` out-param, or rather than directly
 * returning the `T` and throwing an exception on error. StatusOr is used so
 * that callers can choose whether they want to explicitly check for errors,
 * crash the program, or throw exceptions. Since constructors do not have a
 * return value, they should be designed in such a way that they cannot fail by
 * moving the object's complex initialization logic into a separate factory
 * function that itself can return a `StatusOr<T>`. For example:
 *
 * @code
 * class Bar {
 *  public:
 *   Bar(Arg arg);
 *   ...
 * };
 * StatusOr<Bar> MakeBar() {
 *   ... complicated logic that might fail
 *   return Bar(std::move(arg));
 * }
 * @endcode
 *
 * TODO(...) - the current implementation is fairly naive with respect to `T`,
 *   it is unlikely to work correctly for reference types, arrays, and so forth.
 *
 * @tparam T the type of the value.
 */
template<typename T>
class StatusOr final {
 public:
  /**
   * StatusOr is not default constructible.
   *
   * There is no good definition of a default StatusOr:
   * it can't default construct T because T may not have a default constructor,
   * and using an unknown error code and generic message is not helpful.
   */
  StatusOr() = delete;

  /**
   * Creates a new `StatusOr<T>` holding the error condition @p rhs.
   * Creating a StatusOr from an OK status is not permitted
   * and invokes `std::abort()`.
   *
   * @par Post-conditions
   * `ok() == false` and `status() == rhs`.
   *
   * @param rhs the status to initialize the object.
   */
  StatusOr(Status rhs) : status_(std::move(rhs)) {
    if(status_.IsOk()){
      std::cerr << "Constructing StatusOr<T> from OK status is not allowed" << std::endl;
      std::abort();
    }
  }

  /**
   * Creates a new `StatusOr<T>` holding the value @p rhs.
   *
   * @par Post-conditions
   * `ok() == true` and `value() == rhs`.
   *
   * @param rhs the value used to initialize the object.
   */
  StatusOr(T const& rhs) : status_() {
    new (&value_) T(rhs);
  }

  StatusOr(T&& rhs) : status_() {
    new (&value_) T(std::move(rhs));
  }

  StatusOr(StatusOr const& rhs) : status_(rhs.status_) {
    if(ok()) {
      new (&value_) T(rhs.value_);
    }
  }

  StatusOr(StatusOr&& rhs) : status_(std::move(rhs.status_)) {
    if(ok()) {
      new (&value_) T(std::move(rhs.value_));
    }
  }

  ~StatusOr() {
    if(ok()) {
      value_.~T();
    }
  }

  /**
   * @name Status accessors.
   *
   * @return All these member functions return the (properly ref and
   *     const-qualified) status. Iff the object contains a value then
   *     `status().ok() == true`.
   */
  Status const& status() const { return status_; }

  inline bool ok() const { return status_.IsOk(); }
  explicit inline operator bool() const { return ok(); }

  /**
   * @name Deference operators.
   *
   * @warning Using these operators when `ok() == false` results in undefined
   *     behavior.
   *
   * @return All these return a (properly ref and const-qualified) reference to
   *     the underlying value.
   */
  T& operator*() & { return value_; }
  T const& operator*() const& { return value_; }
  T&& operator*() && { return std::move(value_); }

  /**
   * @name Member access operators.
   *
   * @warning Using these operators when `ok() == false` results in undefined
   *     behavior.
   *
   * @return All these return a (properly ref and const-qualified) pointer to
   *     the underlying value.
   */
  T* operator->() & { return &value_; }
  T const* operator->() const& { return &value_; }

  /**
   * @name Value accessors.
   *
   * @return All these member functions return a (properly ref and
   *     const-qualified) reference to the underlying value.
   *
   * Aborts the program if `!ok()`
   */
  T& value() & {
    check_value();
    return **this;
  }

  T const& value() const& {
    check_value();
    return **this;
  }

  T&& value() && {
    check_value();
    return std::move(**this);
  }

 private:
  void check_value() const {
    if(!ok()) {
      std::cerr << status_ << std::endl;
      std::abort();
    }
  }

  Status const status_;
  union{
    T value_;
  };
};

}  // namespace gax
}  // namespace google

#endif  //  GOOGLE_GAX_STATUS_OR_H_

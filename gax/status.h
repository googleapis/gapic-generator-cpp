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

#ifndef GOOGLE_GAX_STATUS_H_
#define GOOGLE_GAX_STATUS_H_

#include <ostream>
#include <string>

namespace google {
namespace gax {

enum class StatusCode {
  /// Not an error; returned on success.
  kOk = 0,

  kCancelled = 1,
  kUnknown = 2,
  kInvalidArgument = 3,
  kDeadlineExceeded = 4,
  kNotFound = 5,
  kAlreadyExists = 6,
  kPermissionDenied = 7,
  kResourceExhausted = 8,
  kFailedPrecondition = 9,
  kAborted = 10,
  kOutOfRange = 11,
  kUnimplemented = 12,
  kInternal = 13,
  kUnavailable = 14,
  kDataLoss = 15,
  kUnauthenticated = 16,
};

class Status {
 public:
  Status() : code_(StatusCode::kOk) {}
  explicit Status(StatusCode code, std::string msg) : code_(code),
                                                      msg_(std::move(msg)) {}

  inline bool IsOk() const { return code_ == StatusCode::kOk; }
  inline bool IsTransientFailure() const {
    return (code_ == StatusCode::kAborted ||
            code_ == StatusCode::kUnavailable ||
            code_ == StatusCode::kDeadlineExceeded);
  }
  inline bool IsPermanentFailure() const { return !IsOk() && !IsTransientFailure(); }

  inline StatusCode code() const { return code_; }

  inline std::string const& message() const { return msg_; }

  bool operator==(Status const& rhs) const {
    return code_ == rhs.code_ && msg_ == rhs.msg_;
  }
  bool operator!=(Status const& rhs) const { return !(*this == rhs); }

 private:
  StatusCode const code_;
  std::string const msg_;
};

std::string StatusCodeToString(StatusCode code);
std::ostream& operator<<(std::ostream& os, StatusCode code);
std::ostream& operator<<(std::ostream& os, Status const& rhs);

}  // namespace gax
}  // namespace google

#endif  // GOOGLE_GAX_RETRY_POLICY_H_

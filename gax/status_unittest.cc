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

#include "googletest/include/gtest/gtest.h"
#include "status.h"

#include <sstream>
#include <string>

namespace {

using namespace ::google;

TEST(Status,Basic){
  {
    gax::Status s;  // sanity check for default construction
    EXPECT_TRUE(s.IsOk());
    EXPECT_FALSE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kCancelled, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kInvalidArgument, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kDeadlineExceeded, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_FALSE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kNotFound, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kAlreadyExists, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kPermissionDenied, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kUnauthenticated, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kResourceExhausted, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kFailedPrecondition, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kAborted, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_FALSE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kOutOfRange, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kUnimplemented, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kInternal, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kUnavailable, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_FALSE(s.IsPermanentFailure());
  }
  {
    gax::Status s(gax::StatusCode::kDataLoss, "");
    EXPECT_FALSE(s.IsOk());
    EXPECT_TRUE(s.IsPermanentFailure());
  }
}

TEST(Status,CodeOstream){
  std::ostringstream output;

  output << gax::StatusCode::kOk;
  EXPECT_EQ("OK", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kCancelled;
  EXPECT_EQ("CANCELLED", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kUnknown;
  EXPECT_EQ("UNKNOWN", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kInvalidArgument;
  EXPECT_EQ("INVALID_ARGUMENT", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kDeadlineExceeded;
  EXPECT_EQ("DEADLINE_EXCEEDED", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kNotFound;
  EXPECT_EQ("NOT_FOUND", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kAlreadyExists;
  EXPECT_EQ("ALREADY_EXISTS", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kPermissionDenied;
  EXPECT_EQ("PERMISSION_DENIED", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kResourceExhausted;
  EXPECT_EQ("RESOURCE_EXHAUSTED", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kFailedPrecondition;
  EXPECT_EQ("FAILED_PRECONDITION", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kAborted;
  EXPECT_EQ("ABORTED", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kOutOfRange;
  EXPECT_EQ("OUT_OF_RANGE", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kUnimplemented;
  EXPECT_EQ("UNIMPLEMENTED", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kInternal;
  EXPECT_EQ("INTERNAL", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kUnavailable;
  EXPECT_EQ("UNAVAILABLE", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kDataLoss;
  EXPECT_EQ("DATA_LOSS", output.str());

  output = std::ostringstream();
  output << gax::StatusCode::kUnauthenticated;
  EXPECT_EQ("UNAUTHENTICATED", output.str());

  output = std::ostringstream();
  output << static_cast<gax::StatusCode>(42);
  EXPECT_EQ("UNEXPECTED_STATUS_CODE=42", output.str());
}

TEST(Status,Ostream) {
  {
    std::ostringstream output;
    gax::Status s;

    output << s;
    EXPECT_EQ(" [OK]", output.str());
  }

  {
    std::ostringstream output;
    gax::Status s(gax::StatusCode::kCancelled, "Because");
    output << s;
    EXPECT_EQ("Because [CANCELLED]", output.str());
  }
}

TEST(Status,Equality) {
  gax::Status ok1, ok2;
  EXPECT_EQ(ok1, ok2);

  gax::Status ok3(gax::StatusCode::kOk, "");
  EXPECT_EQ(ok1, ok3);

  gax::Status ok4(gax::StatusCode::kOk, "Because");
  EXPECT_NE(ok1, ok4);

  gax::Status cancelled1(gax::StatusCode::kCancelled, "");
  EXPECT_NE(ok1, cancelled1);

  gax::Status cancelled2(gax::StatusCode::kCancelled, "Because");
  EXPECT_NE(ok1, cancelled2);
}

}  // namespace

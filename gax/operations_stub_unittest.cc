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

#include "operations_stub.h"
#include "call_context.h"
#include "google/longrunning/operations.pb.h"
#include "googletest/include/gtest/gtest.h"
#include "status.h"

namespace {

using namespace ::google;

class DummyOperationsStub : public gax::OperationsStub {
 public:
  ~DummyOperationsStub() = default;
};

TEST(OperationsStub, Basic) {
  DummyOperationsStub stub{};

  longrunning::GetOperationRequest getOpReq;
  gax::CallContext getCtx(gax::OperationsStub::get_operation_info_);
  EXPECT_EQ(stub.GetOperation(getCtx, getOpReq, nullptr),
            gax::Status(gax::StatusCode::kUnimplemented,
                        "GetOperation not implemented"));

  longrunning::DeleteOperationRequest delOpReq;
  gax::CallContext delCtx(gax::OperationsStub::delete_operation_info_);
  EXPECT_EQ(stub.DeleteOperation(delCtx, delOpReq, nullptr),
            gax::Status(gax::StatusCode::kUnimplemented,
                        "DeleteOperation not implemented"));

  longrunning::CancelOperationRequest canOpReq;
  gax::CallContext cancelCtx(gax::OperationsStub::cancel_operation_info_);
  EXPECT_EQ(stub.CancelOperation(cancelCtx, canOpReq, nullptr),
            gax::Status(gax::StatusCode::kUnimplemented,
                        "CancelOperation not implemented"));
}

}  // namespace

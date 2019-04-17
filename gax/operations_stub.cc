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

#include "google/longrunning/operations.pb.h"

#include "call_context.h"
#include "operations_stub.h"
#include "status.h"

namespace google {
namespace gax {

constexpr MethodInfo OperationsStub::get_operation_info_;
constexpr MethodInfo OperationsStub::delete_operation_info_;
constexpr MethodInfo OperationsStub::cancel_operation_info_;

OperationsStub::~OperationsStub() {}

gax::Status OperationsStub::GetOperation(
    gax::CallContext&, google::longrunning::GetOperationRequest const&,
    google::longrunning::Operation*) {
  return gax::Status{gax::StatusCode::kUnimplemented,
                     "GetOperation not implemented"};
}

gax::Status OperationsStub::DeleteOperation(
    gax::CallContext&, google::longrunning::DeleteOperationRequest const&,
    google::protobuf::Empty*) {
  return gax::Status{gax::StatusCode::kUnimplemented,
                     "DeleteOperation not implemented"};
}

gax::Status OperationsStub::CancelOperation(
    gax::CallContext&, google::longrunning::CancelOperationRequest const&,
    google::protobuf::Empty*) {
  return gax::Status{gax::StatusCode::kUnimplemented,
                     "CancelOperation not implemented"};
}

}  // namespace gax

}  // namespace google

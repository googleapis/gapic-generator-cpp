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

#ifndef GAPIC_GENERATOR_CPP_GAX_OPERATIONS_STUB_H_
#define GAPIC_GENERATOR_CPP_GAX_OPERATIONS_STUB_H_

#include "google/longrunning/operations.pb.h"

#include "call_context.h"
#include "status.h"

namespace google {
namespace gax {

class OperationsStub {
 public:
  virtual ~OperationsStub() = 0;

  virtual gax::Status GetOperation(
      gax::CallContext& context,
      google::longrunning::GetOperationRequest const& request,
      google::longrunning::Operation* response);

  virtual gax::Status DeleteOperation(
      gax::CallContext& context,
      google::longrunning::DeleteOperationRequest const& request,
      google::protobuf::Empty* response);

  virtual gax::Status CancelOperation(
      gax::CallContext& context,
      google::longrunning::CancelOperationRequest const& request,
      google::protobuf::Empty* response);

  static constexpr MethodInfo get_operation_info_ = {
      "GetOperation", MethodInfo::RpcType::NORMAL_RPC,
      MethodInfo::Idempotency::IDEMPOTENT};
  static constexpr MethodInfo delete_operation_info_ = {
      "DeleteOperation", MethodInfo::RpcType::NORMAL_RPC,
      MethodInfo::Idempotency::IDEMPOTENT};
  static constexpr MethodInfo cancel_operation_info_ = {
      "CancelOperation", MethodInfo::RpcType::NORMAL_RPC,
      MethodInfo::Idempotency::IDEMPOTENT};
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_OPERATIONS_STUB_H_

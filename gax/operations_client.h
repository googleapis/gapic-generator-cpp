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
#ifndef GAPIC_GENERATOR_CPP_GAX_OPERATIONS_CLIENT_H_
#define GAPIC_GENERATOR_CPP_GAX_OPERATIONS_CLIENT_H_

#include "gax/operation.h"
#include "gax/operations_stub.h"
#include "gax/status.h"
#include <memory>
#include <utility>

namespace google {
namespace gax {

class OperationsClient final {
 public:
  OperationsClient(std::shared_ptr<gax::OperationsStub> stub)
      : stub_(std::move(stub)) {}

  /**
   * @name Ping the server and check if the operation has finished.
   * Updates
   * metadata.
   *
   * @return a status indicating whether the update was successful.
   */
  template <typename ResultT, typename MetadataT>
  gax::Status Update(gax::Operation<ResultT, MetadataT>& op) {
    if (!op.Done()) {
      google::longrunning::GetOperationRequest request;
      google::longrunning::Operation tmp;
      request.set_name(op.Name());
      gax::CallContext context(get_operation_info);
      auto status = stub_->GetOperation(context, request, &tmp);

      if (status.IsOk()) {
        op = std::move(tmp);
      }

      return status;
    } else {
      return gax::Status{};
    }
  }

  /**
   * @name Inform the service that the client is no longer interested in the
   * result.
   *
   * @return a status indicating whether the rpc was successful.
   */
  template <typename ResultT, typename MetadataT>
  gax::Status Delete(gax::Operation<ResultT, MetadataT> const& op) {
    google::longrunning::DeleteOperationRequest request;
    google::protobuf::Empty empty;
    request.set_name(op.Name());
    gax::CallContext context(delete_operation_info);
    return stub_->DeleteOperation(context, request, &empty);
  }

  /**
   * @name Best-effort cancellation attempt of the operation.
   *
   * @return a status indicating whether the rpc was successful.
   */
  template <typename ResultT, typename MetadataT>
  gax::Status Cancel(gax::Operation<ResultT, MetadataT> const& op) {
    google::longrunning::CancelOperationRequest request;
    google::protobuf::Empty empty;
    request.set_name(op.Name());
    gax::CallContext context(cancel_operation_info);
    return stub_->CancelOperation(context, request, &empty);
  }

  static constexpr MethodInfo get_operation_info = {
      "GetOperation", MethodInfo::RpcType::NORMAL_RPC,
      MethodInfo::Idempotency::IDEMPOTENT};
  static constexpr MethodInfo delete_operation_info = {
      "DeleteOperation", MethodInfo::RpcType::NORMAL_RPC,
      MethodInfo::Idempotency::IDEMPOTENT};
  static constexpr MethodInfo cancel_operation_info = {
      "CancelOperation", MethodInfo::RpcType::NORMAL_RPC,
      MethodInfo::Idempotency::IDEMPOTENT};

 private:
  std::shared_ptr<gax::OperationsStub> stub_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_OPERATIONS_CLIENT_H_

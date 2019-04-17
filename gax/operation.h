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

#ifndef GAPIC_GENERATOR_CPP_GAX_OPERATION_H_
#define GAPIC_GENERATOR_CPP_GAX_OPERATION_H_

#include "call_context.h"
#include "internal/gtest_prod.h"
#include "operations_stub.h"
#include "status.h"
#include "status_or.h"

#include "google/longrunning/operations.pb.h"

#include <iostream>

#include <memory>

namespace google {

namespace gax {

// Note: compilation will fail unless ResponseT and MetadataT are children of
// protobuf::Message.
template <typename ResponseT, typename MetadataT>
class Operation final {
 public:
  Operation(std::shared_ptr<gax::OperationsStub> stub,
            google::longrunning::Operation op)
      : stub_(std::move(stub)), op_(std::move(op)) {}
  Operation() = delete;
  Operation(Operation&& rhs) = default;

  std::string const& Name() { return op_.name(); }

  gax::StatusOr<ResponseT> Result() {
    if (!Done()) {
      return gax::Status{gax::StatusCode::kUnknown,
                         "operation has not completed=" + Name()};
    } else if (op_.has_error()) {
      return gax::Status{static_cast<gax::StatusCode>(op_.error().code()),
                         op_.error().message()};
    } else {
      auto const& any = op_.response();
      if (!any.Is<ResponseT>()) {
        return gax::Status{gax::StatusCode::kUnknown,
                           "invalid result in operation=" + Name()};
      }

      ResponseT result;
      any.UnpackTo(&result);
      return std::move(result);
    }
  }

  MetadataT Metadata() const {
    MetadataT m;
    op_.metadata().UnpackTo(&m);
    return m;
  }

  gax::Status Update() {
    if (!Done()) {
      google::longrunning::GetOperationRequest request;
      request.set_name(op_.name());
      gax::CallContext context(OperationsStub::get_operation_info_);
      gax::Status result = stub_->GetOperation(context, request, &op_);
      return result;
    } else if (op_.has_error()) {
      return gax::Status{static_cast<gax::StatusCode>(op_.error().code()),
                         op_.error().message()};
    } else {
      return gax::Status{};
    }
  }

  gax::Status Delete() {
    google::longrunning::DeleteOperationRequest request;
    google::protobuf::Empty empty;
    request.set_name(op_.name());
    gax::CallContext context(OperationsStub::delete_operation_info_);
    return stub_->DeleteOperation(context, request, &empty);
  }

  gax::Status Cancel() {
    google::longrunning::CancelOperationRequest request;
    google::protobuf::Empty empty;
    request.set_name(op_.name());
    gax::CallContext context(OperationsStub::cancel_operation_info_);
    return stub_->CancelOperation(context, request, &empty);
  }

  bool Done() { return op_.done(); }

 private:
  FRIEND_TEST(Operation, Basic);
  FRIEND_TEST(Operation, Update);
  FRIEND_TEST(Operation, Result);
  FRIEND_TEST(Operation, Metadata);

  std::shared_ptr<gax::OperationsStub> stub_;
  google::longrunning::Operation op_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_OPERATION_H_

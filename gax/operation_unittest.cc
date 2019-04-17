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

#include "operation.h"
#include "status.h"

#include "google/longrunning/operations.pb.h"
#include "googletest/include/gtest/gtest.h"

#include <memory>
#include <string>
#include <type_traits>

namespace google {
namespace gax {

class DummyOperationsStub : public gax::OperationsStub {
 public:
  ~DummyOperationsStub() = default;

  gax::Status GetOperation(
      gax::CallContext& context,
      google::longrunning::GetOperationRequest const& request,
      google::longrunning::Operation* response) override {
    auto info = context.Info();
    auto get_info = OperationsStub::get_operation_info_;
    EXPECT_EQ(std::string(info.rpc_name), std::string(get_info.rpc_name));
    EXPECT_EQ(info.rpc_type, get_info.rpc_type);
    EXPECT_EQ(info.idempotency, get_info.idempotency);

    EXPECT_EQ(request.name(), "test");
    EXPECT_NE(response, nullptr);
    EXPECT_EQ(response->name(), "test");
    has_gotten = true;
    return gax::Status{};
  }
  gax::Status DeleteOperation(
      gax::CallContext& context,
      google::longrunning::DeleteOperationRequest const& request,
      google::protobuf::Empty* response) override {
    auto info = context.Info();
    auto delete_info = OperationsStub::delete_operation_info_;
    EXPECT_EQ(std::string(info.rpc_name), std::string(delete_info.rpc_name));
    EXPECT_EQ(info.rpc_type, delete_info.rpc_type);
    EXPECT_EQ(info.idempotency, delete_info.idempotency);

    EXPECT_EQ(request.name(), "test");
    EXPECT_NE(response, nullptr);
    has_deleted = true;
    return gax::Status{};
  }

  gax::Status CancelOperation(
      gax::CallContext& context,
      google::longrunning::CancelOperationRequest const& request,
      google::protobuf::Empty* response) override {
    auto info = context.Info();
    auto cancel_info = OperationsStub::cancel_operation_info_;
    EXPECT_EQ(std::string(info.rpc_name), std::string(cancel_info.rpc_name));
    EXPECT_EQ(info.rpc_type, cancel_info.rpc_type);
    EXPECT_EQ(info.idempotency, cancel_info.idempotency);

    EXPECT_EQ(request.name(), "test");
    EXPECT_NE(response, nullptr);
    has_canceled = true;
    return gax::Status{};
  }

  static void clear_flags() {
    has_gotten = false;
    has_deleted = false;
    has_canceled = false;
  }

  static bool has_gotten;
  static bool has_deleted;
  static bool has_canceled;
};

bool DummyOperationsStub::has_gotten;
bool DummyOperationsStub::has_deleted;
bool DummyOperationsStub::has_canceled;

static_assert(!std::is_default_constructible<gax::Operation<int, int>>::value,
              "Operation should not be default-constructible.");
static_assert(!std::is_copy_constructible<gax::Operation<int, int>>::value,
              "Operation should be move-constructible.");
static_assert(std::is_move_constructible<gax::Operation<int, int>>::value,
              "Operation should be move-constructible.");

// Using GetOperationRequest as the Result and Metadata types to prevent
// dependencies on additional proto libraries.
TEST(Operation, Basic) {
  google::longrunning::Operation lro;
  lro.set_name("test");
  gax::Operation<int, int> op(
      std::shared_ptr<gax::OperationsStub>(new DummyOperationsStub()),
      std::move(lro));

  EXPECT_EQ(op.Name(), "test");
  EXPECT_EQ(DummyOperationsStub::has_deleted, false);
  op.Delete();
  EXPECT_EQ(DummyOperationsStub::has_deleted, true);

  EXPECT_EQ(op.Name(), "test");
  EXPECT_EQ(DummyOperationsStub::has_canceled, false);
  op.Cancel();
  EXPECT_EQ(DummyOperationsStub::has_canceled, true);

  EXPECT_EQ(op.Done(), false);
  op.op_.set_done(true);
  EXPECT_EQ(op.Done(), true);
}

TEST(Operation, Update) {
  google::longrunning::Operation lro;
  lro.set_name("test");

  gax::Operation<int, google::longrunning::GetOperationRequest> op(
      std::shared_ptr<gax::OperationsStub>(new DummyOperationsStub()),
      std::move(lro));

  EXPECT_EQ(DummyOperationsStub::has_gotten, false);
  op.Update();
  EXPECT_EQ(DummyOperationsStub::has_gotten, true);

  DummyOperationsStub::clear_flags();
  op.op_.set_done(true);
  op.op_.mutable_error()->set_code(5);
  op.op_.mutable_error()->set_message("This is an error message");
  auto fail_res = op.Update();
  EXPECT_EQ(DummyOperationsStub::has_gotten, false);
  EXPECT_EQ(fail_res, gax::Status(gax::StatusCode::kNotFound,
                                  "This is an error message"));

  op.op_.clear_error();
  auto passthrough_res = op.Update();
  EXPECT_EQ(DummyOperationsStub::has_gotten, false);
  EXPECT_EQ(passthrough_res, gax::Status{});
}

TEST(Operation, Result) {
  google::longrunning::Operation lro;
  lro.set_name("test");

  gax::Operation<google::longrunning::GetOperationRequest,
                 google::longrunning::GetOperationRequest>
      op(std::shared_ptr<gax::OperationsStub>(new DummyOperationsStub()),
         std::move(lro));
  auto incomplete_res = op.Result();
  EXPECT_EQ(bool(incomplete_res), false);
  EXPECT_EQ(incomplete_res.status(),
            gax::Status(gax::StatusCode::kUnknown,
                        "operation has not completed=test"));

  op.op_.set_done(true);
  op.op_.mutable_error()->set_code(5);
  op.op_.mutable_error()->set_message("This is an error message");
  auto error_res = op.Result();
  EXPECT_EQ(bool(error_res), false);
  EXPECT_EQ(error_res.status(), gax::Status(gax::StatusCode::kNotFound,
                                            "This is an error message"));

  google::longrunning::GetOperationRequest dummy;
  dummy.set_name("dummy-response");
  op.op_.clear_error();
  op.op_.set_done(true);
  op.op_.mutable_response()->PackFrom(dummy);

  dummy.Clear();
  auto success_res = op.Result();
  EXPECT_EQ(bool(success_res), true);
  EXPECT_EQ((*success_res).name(), "dummy-response");

  google::longrunning::CancelOperationRequest dud;
  dud.set_name("dud-response");
  op.op_.clear_response();
  op.op_.mutable_response()->PackFrom(dud);
  dud.Clear();
  auto bad_result = op.Result();
  EXPECT_EQ(bool(bad_result), false);
  EXPECT_EQ(bad_result.status(),
            gax::Status(gax::StatusCode::kUnknown,
                        "invalid result in operation=test"));
}

TEST(Operation, Metadata) {
  google::longrunning::Operation lro;
  lro.set_name("test");

  gax::Operation<int, google::longrunning::GetOperationRequest> op(
      std::shared_ptr<gax::OperationsStub>(new DummyOperationsStub()),
      std::move(lro));

  google::longrunning::GetOperationRequest dummy;
  dummy.set_name("dummy-metadata");
  op.op_.mutable_metadata()->PackFrom(dummy);
  dummy.Clear();
  google::longrunning::GetOperationRequest metadata = op.Metadata();
  EXPECT_EQ(metadata.name(), "dummy-metadata");

  // Unpacking from the wrong type is a no-op.
  metadata.Clear();
  google::longrunning::CancelOperationRequest dud;
  dud.set_name("dud-metadata");
  op.op_.mutable_metadata()->PackFrom(dud);
  dud.Clear();
  metadata = op.Metadata();
  EXPECT_EQ(metadata.name(), "");
}

}  // namespace gax
}  // namespace google

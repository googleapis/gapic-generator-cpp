// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gax/operation.h"
#include "google/longrunning/operations.pb.h"
#include "gax/operations_client.h"
#include "gax/operations_stub.h"
#include "gax/status.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>

namespace google {
namespace gax {

using TestOperation = gax::Operation<google::longrunning::GetOperationRequest,
                                     google::longrunning::GetOperationRequest>;

class DummyOperationsStub : public gax::OperationsStub {
 public:
  ~DummyOperationsStub() = default;

  gax::Status GetOperation(
      gax::CallContext& context,
      google::longrunning::GetOperationRequest const& request,
      google::longrunning::Operation* response) override {
    auto info = context.Info();
    auto get_info = OperationsClient::get_operation_info;
    EXPECT_EQ(std::string(info.rpc_name), std::string(get_info.rpc_name));
    EXPECT_EQ(info.rpc_type, get_info.rpc_type);
    EXPECT_EQ(info.idempotency, get_info.idempotency);

    EXPECT_NE(response, nullptr);
    response->set_name("test-response");
    response->set_done(set_done);

    if (pack_correctly) {
      google::longrunning::GetOperationRequest dummy;
      dummy.set_name("dummy-response");
      response->mutable_response()->PackFrom(dummy);
    } else {
      google::longrunning::CancelOperationRequest dud;
      dud.set_name("dud-response");
      response->mutable_response()->PackFrom(dud);
    }

    if (pack_metadata_correctly) {
      google::longrunning::GetOperationRequest dummy;
      dummy.set_name("dummy-metadata");
      response->mutable_metadata()->PackFrom(dummy);
    } else {
      google::longrunning::CancelOperationRequest dud;
      dud.set_name("dud-metadata");
      response->mutable_metadata()->PackFrom(dud);
    }

    if (set_error) {
      response->mutable_error()->set_code(5);  // kNotFound
      response->mutable_error()->set_message("This is an error message");
    } else {
      response->clear_error();
    }

    has_gotten = true;
    return return_ok ? gax::Status{}
                     : gax::Status{gax::StatusCode::kUnknown, "Unknown error"};
  }

  gax::Status DeleteOperation(
      gax::CallContext& context,
      google::longrunning::DeleteOperationRequest const& request,
      google::protobuf::Empty* response) override {
    auto info = context.Info();
    auto delete_info = OperationsClient::delete_operation_info;
    EXPECT_EQ(std::string(info.rpc_name), std::string(delete_info.rpc_name));
    EXPECT_EQ(info.rpc_type, delete_info.rpc_type);
    EXPECT_EQ(info.idempotency, delete_info.idempotency);

    EXPECT_EQ(request.name(), "test");
    EXPECT_NE(response, nullptr);
    has_deleted = true;
    return return_ok ? gax::Status{}
                     : gax::Status{gax::StatusCode::kUnknown, "Unknown error"};
  }

  gax::Status CancelOperation(
      gax::CallContext& context,
      google::longrunning::CancelOperationRequest const& request,
      google::protobuf::Empty* response) override {
    auto info = context.Info();
    auto cancel_info = OperationsClient::cancel_operation_info;
    EXPECT_EQ(std::string(info.rpc_name), std::string(cancel_info.rpc_name));
    EXPECT_EQ(info.rpc_type, cancel_info.rpc_type);
    EXPECT_EQ(info.idempotency, cancel_info.idempotency);

    EXPECT_EQ(request.name(), "test");
    EXPECT_NE(response, nullptr);
    has_canceled = true;
    return return_ok ? gax::Status{}
                     : gax::Status{gax::StatusCode::kUnknown, "Unknown error"};
  }

  bool has_gotten;
  bool has_deleted;
  bool has_canceled;
  bool return_ok = true;
  bool set_done;
  bool set_error;
  bool pack_correctly = true;
  bool pack_metadata_correctly = true;
  std::string response_str = "response";
};

static_assert(!std::is_default_constructible<TestOperation>::value,
              "Operation should not be default-constructible.");
static_assert(std::is_copy_constructible<TestOperation>::value,
              "Operation should be copy-constructible.");
static_assert(std::is_copy_assignable<TestOperation>::value,
              "Operation should be copy-assignable.");
static_assert(std::is_move_constructible<TestOperation>::value,
              "Operation should be move-constructible.");
static_assert(std::is_move_assignable<TestOperation>::value,
              "Operation should be move-assignable.");

// Using GetOperationRequest as the Result and Metadata types to prevent
// dependencies on additional proto libraries.
TEST(Operation, Basic) {
  google::longrunning::Operation lro;
  std::shared_ptr<DummyOperationsStub> stub(new DummyOperationsStub());
  gax::OperationsClient client(stub);
  lro.set_name("test");
  TestOperation op(std::move(lro));

  EXPECT_EQ(op.Name(), "test");

  EXPECT_EQ(stub->has_deleted, false);
  client.Delete(op);
  EXPECT_EQ(stub->has_deleted, true);

  EXPECT_EQ(stub->has_canceled, false);
  client.Cancel(op);
  EXPECT_EQ(stub->has_canceled, true);
}

TEST(Operation, Update) {
  google::longrunning::Operation lro;
  std::shared_ptr<DummyOperationsStub> stub(new DummyOperationsStub());
  gax::OperationsClient client(stub);
  lro.set_name("test");

  TestOperation op(std::move(lro));

  stub->return_ok = false;
  EXPECT_EQ(stub->has_gotten, false);
  auto fail_res = client.Update(op);
  EXPECT_EQ(fail_res, gax::Status(gax::StatusCode::kUnknown, "Unknown error"));
  // This is a mild hack to see whether the underlying operation is preserved if
  // the rpc returned an error status.
  EXPECT_EQ(op.Name(), "test");
  EXPECT_EQ(stub->has_gotten, true);

  stub->set_done = true;
  stub->return_ok = true;
  auto succeed_res = client.Update(op);
  EXPECT_EQ(succeed_res, gax::Status{});
  EXPECT_EQ(op.Name(), "test-response");
  EXPECT_TRUE(op.Done());

  stub->has_gotten = false;
  stub->return_ok = false;
  stub->response_str = "secondary-response";
  auto passthrough_success = client.Update(op);
  // Three way safety check:
  // * Invoking GetOperation should set has_gotten
  // * The stub rpc should return a non-ok status if invoked
  // * The stub rpc should set a new name if invoked.
  EXPECT_EQ(passthrough_success, gax::Status{});
  EXPECT_EQ(stub->has_gotten, false);
  EXPECT_EQ(op.Name(), "test-response");
}

TEST(Operation, Result) {
  std::shared_ptr<DummyOperationsStub> stub(new DummyOperationsStub());
  gax::OperationsClient client(stub);

  google::longrunning::Operation lro;
  lro.set_name("test");
  TestOperation op(std::move(lro));
  auto incomplete_res = op.Result();
  EXPECT_EQ(static_cast<bool>(incomplete_res), false);
  EXPECT_EQ(incomplete_res.status(),
            gax::Status(gax::StatusCode::kUnknown,
                        "operation has not completed=test"));

  stub->set_done = true;
  stub->set_error = true;
  client.Update(op);
  EXPECT_TRUE(op.Done());
  auto error_res = op.Result();
  EXPECT_EQ(static_cast<bool>(error_res), false);
  EXPECT_EQ(error_res.status(), gax::Status(gax::StatusCode::kNotFound,
                                            "This is an error message"));

  lro.set_name("test");
  op = TestOperation(std::move(lro));
  stub->set_error = false;
  client.Update(op);
  auto success_res = op.Result();
  EXPECT_EQ(static_cast<bool>(success_res), true);
  EXPECT_EQ((*success_res).name(), "dummy-response");

  lro.set_name("test");
  stub->pack_correctly = false;
  op = TestOperation(std::move(lro));
  client.Update(op);
  auto bad_result = op.Result();
  EXPECT_EQ(static_cast<bool>(bad_result), false);
  EXPECT_EQ(bad_result.status(),
            gax::Status(gax::StatusCode::kUnknown,
                        "invalid result in operation=test-response"));
}

TEST(Operation, Metadata) {
  google::longrunning::Operation lro;
  lro.set_name("test");
  std::shared_ptr<DummyOperationsStub> stub(new DummyOperationsStub());
  gax::OperationsClient client(stub);

  TestOperation op(std::move(lro));

  client.Update(op);
  google::longrunning::GetOperationRequest metadata = op.Metadata();
  EXPECT_EQ(metadata.name(), "dummy-metadata");

  stub->pack_metadata_correctly = false;
  client.Update(op);
  metadata = op.Metadata();
  EXPECT_EQ(metadata.name(), "");
}

}  // namespace gax
}  // namespace google

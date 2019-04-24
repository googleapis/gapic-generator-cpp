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

#ifndef GAPIC_GENERATOR_CPP_GAX_CALL_CONTEXT_H_
#define GAPIC_GENERATOR_CPP_GAX_CALL_CONTEXT_H_

#include "grpcpp/client_context.h"
#include "internal/gtest_prod.h"

#include <chrono>
#include <functional>
#include <map>
#include <string>
#include <utility>

/**
 * Callback type for custom manipulation of grpc::ClientContext.
 * These callbacks can be used to manipulate advanced grpc settings.
 */
using GrpcContextPolicyFunc = std::function<void(grpc::ClientContext*)>;

namespace google {
namespace gax {

/**
 * Compile time information about specific rpc methods.
 * This information can be used by user provided GAPIC stub decorator methods to
 * configure ClientContexts.
 *
 * Example:
 * @code
 * gax::Status GetFoo(gax::CallContext& context, GetFooRequest const& request,
 *                    Foo* response) {
 *   auto info = context.Info();
 *   if(info.idempotency == gax::MethodInfo::Idempotency::IDEMPOTENCY) {
 *     context.addGrpcContextPolicy([](grpc::ClientContext* ctx){
 *                                       ctx->set_idempotent(true);
 *                                     });
 *   }
 *   return inner_stub_->GetFoo(context, request, response);
 * }
 *
 */
struct MethodInfo {
  enum class RpcType {
    // Copied from grpc include/grpcpp/impl/codegen/rpc_method.h#L32
    NORMAL_RPC = 0,
    CLIENT_STREAMING,
    SERVER_STREAMING,
    BIDI_STREAMING
  };

  enum class Idempotency { NON_IDEMPOTENT = 0, IDEMPOTENT = 1 };

  char const* const rpc_name;
  RpcType const rpc_type;
  Idempotency const idempotency;
};

/**
 * Reusable, customizable initializer for grpc::ClientContext.
 *
 * grpc::ClientContext is not movable, copyable, or reset-able, and a different
 * ClientContext MUST be used for each rpc invocation. The user may want to
 * customize the context in order to add application-specific metadata, or
 * manipulate advanced knobs; these two requirements complicate the client level
 * retry loop.
 *
 * CallContext is a solution that allows arbitrary user customization and
 * initializes ClientContexts. GAPIC stub layers can customize CallContext, and
 * the stub layer that initiates an actual rpc has an interface that generates
 * the correct ClientContext.
 *
 * Example usage, with both customization and rpc invocation:
 *
 * @code
 * gax::Status OuterStub::GetFoo(gax::CallContext& context,
 *                               GetFooRequest const& request,
 *                               Foo* response) {
 *   int counter = 0;
 *   context.AddGrpcContextPolicy([&counter, &context](grpc::ClientContext* c){
 *       counter++;
 *       c->set_cacheable(true);
 *       c->set_idempotent(context.Info().idempotency ==
 *                         google::gax::MethodInfo::Idempotency::IDEMPOTENT);
 *     });
 *   context.AddMetadata("fooKey", "fooVal");
 *   // Assume retry_stub_ engages in some kind of retry, holds a ptr to an
 *   // InnerStub, and calls inner_stub_->GetFoo.
 *   gax::Status status = retry_stub_->GetFoo(context, request, response);
 *   std::cout << "Needed " << counter << " attempts for GetFoo" << std::endl;
 *   return status;
 * }
 *
 * gax::Status InnerStub::GetFoo(gax::CallContext& context,
 *                               GetFooRequest const& request,
 *                               Foo* response) {
 *   grpc::ClientContext c;
 *   context.PrepareGrpcContext(&c);
 *   return grpc_stub_->GetFoo(&c, request, response);
 * }
 */
class CallContext {
 public:
  CallContext(MethodInfo method_info)
      : deadline_(std::chrono::system_clock::time_point::max()),
        method_info_(std::move(method_info)) {}

  /**
   * Register an arbitrary customization function on grpc::ClientContext.
   * This function could tweak advanced knobs or provide other custom behavior.
   */
  void AddGrpcContextPolicy(GrpcContextPolicyFunc f);

  /**
   * Initializes a grpc::ClientContext, adding stored metadata, setting a
   * deadline, and applying registered customization functions.
   *
   * @par Pre-conditions
   * The 'context' parameter has not been used for an rpc.
   *
   * @param context the context to initialize.
   */
  void PrepareGrpcContext(grpc::ClientContext* context);

  /**
   * @brief Register application-specific metadata.
   *
   * @param key the metadata key.
   * @param val the metadata value.
   */
  void AddMetadata(std::string key, std::string val);

  /**
   * @brief Set a deadline for the rpc.
   */
  void SetDeadline(std::chrono::system_clock::time_point deadline);

  /**
   * @brief Accessor for configured rpc deadline.
   */
  std::chrono::system_clock::time_point Deadline() const;

  /**
   * @brief Accessor for method info.
   */
  MethodInfo Info() const;

 private:
  FRIEND_TEST(CallContext, Basic);
  std::chrono::system_clock::time_point deadline_;
  std::vector<GrpcContextPolicyFunc> context_policies_;
  std::multimap<std::string, std::string const> metadata_;
  MethodInfo const method_info_;
};
}
}

#endif  // GAPIC_GENERATOR_CPP_GAX_CALL_CONTEXT_H_

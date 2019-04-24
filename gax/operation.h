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

#include "google/longrunning/operations.pb.h"
#include "gax/call_context.h"
#include "gax/operations_stub.h"
#include "gax/status.h"
#include "gax/status_or.h"
#include <memory>

namespace google {
namespace gax {

/**
 * Operation is a user-visible primitive used to support Long Running Operations
 * (LROs).
 *
 * LROs are service methods that may take a 'long time' (anywhere from
 * seconds to days), and instead of blocking for that long return an Operation
 * object or an error status. The Operation may contain a result and always
 * contains metadata.
 *
 * A service method that returns `google.longrunning.Operation` and that has
 * the correct annotations for its result and metadata types will cause the
 * generator to define a client method that returns
 * gax::StatusOr<gax::Operation<ResponseT, MetadataT>>.
 *
 * Example usage:
 *
 * @code
 * gax::StatusOr<gax::Operation<Foo, FooMeta>> res =
 * client.GetBigFoo(getBigFooRequest);
 * if(!res) {
 *   ...
 * }
 *
 * operationsClient = client.OperationsClient();
 * Operation<Foo, FooMeta> op = *std::move(res);
 * while(!op.Done()) {
 *   gax::Status stat = operationsClient.Update(op);
 *   if(isPermanentFailure(stat)) {
 *     ...
 *   }
 *   std::cout << "Foo request " << getBigFooRequest().name() << " is " <<
 *             op.Metadata().percent() << "% done.";
 *   std::this_thread::sleep(sleep_period());
 * }
 *
 * gax::StatusOr<Result> terminus = op.Result();
 * if(!terminus) {
 *   ...
 * }
 * Result r = *std::move(terminus);
 * @endcode
 */
template <
    typename ResponseT, typename MetadataT,
    typename std::enable_if<
        std::is_base_of<protobuf::Message, ResponseT>::value, int>::type = 0,
    typename std::enable_if<
        std::is_base_of<protobuf::Message, MetadataT>::value, int>::type = 0>
class Operation final {
 public:
  // Note: the constructor is intended to be used by GAPIC generated code, not
  // users.
  explicit Operation(google::longrunning::Operation op) : op_(std::move(op)) {}

  /**
   * @brief Return the service-provided name of the underlying
   * google.longrunning.Operation
   */
  std::string const& Name() const { return op_.name(); }

  /**
   * @brief If Operation::Done(), return the underlying Response, or an error
   * code if a problem occurred. Otherwise return an error indicating that the
   * Operation has not completed.
   */
  gax::StatusOr<ResponseT> Result() const {
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

  /**
   * @brief Return the most recent metadata received from the service.
   *
   * The metadata type is application specific. Manipulating it is left to the
   * user.
   *
   * @return the most recent metadata.
   */
  MetadataT Metadata() const {
    MetadataT m;
    op_.metadata().UnpackTo(&m);
    return m;
  }

  /**
   * @brief Indicate whether the operation has completed. If true, the Operation
   * now contains a result or an error status.
   */
  bool Done() const { return op_.done(); }

 private:
  google::longrunning::Operation op_;
};

}  // namespace gax
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GAX_OPERATION_H_

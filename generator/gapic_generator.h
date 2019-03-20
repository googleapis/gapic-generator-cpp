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
#ifndef GAPIC_GENERATOR_CPP_GENERATOR_GAPIC_GENERATOR_H_
#define GAPIC_GENERATOR_CPP_GENERATOR_GAPIC_GENERATOR_H_

#include <memory>
#include <sstream>
#include <string>

#include "src/google/protobuf/compiler/code_generator.h"
#include "src/google/protobuf/descriptor.h"

namespace google {
namespace api {
namespace codegen {

/**
 * Code generator that outputs a C++ GAPIC client from proto descriptors.
 */
class GapicGenerator : public google::protobuf::compiler::CodeGenerator {
 public:
  bool Generate(google::protobuf::FileDescriptor const* file,
                std::string const& parameter,
                google::protobuf::compiler::GeneratorContext* generator_context,
                std::string* error) const override;
};

}  // namespace codegen
}  // namespace api
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GENERATOR_GAPIC_GENERATOR_H_

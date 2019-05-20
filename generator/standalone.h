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

#ifndef GAPIC_GENERATOR_CPP_GENERATOR_STANDALONE_H_
#define GAPIC_GENERATOR_CPP_GENERATOR_STANDALONE_H_

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/plugin.pb.h>

namespace google {
namespace api {
namespace codegen {

int StandaloneMain(int argc, const char* const argv[],
                   google::protobuf::compiler::CodeGenerator* generator);

int StandaloneMain(const std::vector<std::string>& descriptors,
                   const std::string& package, const std::string& output,
                   google::protobuf::compiler::CodeGenerator* generator);

}  // namespace codegen
}  // namespace api
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GENERATOR_STANDALONE_H_

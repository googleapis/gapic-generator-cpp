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
#ifndef GOOGLE_API_CODEGEN_GAPIC_GENERATOR_H_
#define GOOGLE_API_CODEGEN_GAPIC_GENERATOR_H_

#include <memory>
#include <sstream>
#include <string>

#include "src/google/protobuf/compiler/code_generator.h"
#include "src/google/protobuf/io/printer.h"
#include "src/google/protobuf/io/zero_copy_stream.h"
#include "src/google/protobuf/descriptor.h"
#include "src/google/protobuf/descriptor.pb.h"
#include "absl/strings/string_view.h"

#include "internal/gapic_template.h"

namespace pb = google::protobuf;

namespace google {
namespace api {
namespace codegen {

class GapicGenerator : public pb::compiler::CodeGenerator {
 public:
  GapicGenerator()
      : template_(internal::CreateDefaultGapicTemplate()) {}

  bool Generate(pb::FileDescriptor const* file,
                std::string const& parameter,
                pb::compiler::GeneratorContext* generator_context,
                std::string* error) const override;

 private:
  std::shared_ptr<internal::GapicTemplate> template_;
  bool GenerateClientHeaderFile(
      pb::ServiceDescriptor const* service,
      std::map<std::string, std::string>& vars,
      pb::io::Printer* printer,
      std::string* error) const;
};

} // namespace codegen
} // namespace api
} // namespace google

#endif // GOOGLE_API_CODEGEN_GAPIC_GENERATOR_H_

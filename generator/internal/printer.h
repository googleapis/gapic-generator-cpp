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
#ifndef GAPIC_GENERATOR_CPP_GENERATOR_INTERNAL_PRINTER_H_
#define GAPIC_GENERATOR_CPP_GENERATOR_INTERNAL_PRINTER_H_

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <string>

namespace pb = google::protobuf;

namespace google {
namespace api {
namespace codegen {
namespace internal {

/**
 * Wrapper around a google::protobuf::io::ZeroCopyOutputStream and a
 * google::protobuf::io::Printer object so that they can be used for
 * code generation.
 */
class Printer {
 public:
  Printer(pb::compiler::GeneratorContext* generator_context,
          std::string const& file_name)
      : output_(generator_context->Open(file_name)),
        printer_(output_.get(), '$', NULL) {}

  pb::io::Printer* operator->() & { return &printer_; }
  pb::io::Printer const* operator->() const& { return &printer_; }

  Printer(Printer const&) = delete;
  Printer& operator=(Printer const&) = delete;

 private:
  std::unique_ptr<pb::io::ZeroCopyOutputStream> output_;
  pb::io::Printer printer_;
};

}  // namespace internal
}  // namespace codegen
}  // namespace api
}  // namespace google

#endif  // GAPIC_GENERATOR_CPP_GENERATOR_INTERNAL_PRINTER_H_

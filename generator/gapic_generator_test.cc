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

#include <gtest/gtest.h>
#include <fstream>
#include <gapic_generator.h>
#include <iostream>
#include <memory>
#include <standalone.h>
#include <string>
#include <vector>

namespace google {
namespace api {
namespace codegen {

namespace pb = google::protobuf;

inline std::string LoadContent(std::string const& f) {
  std::ifstream ifs(f);
  EXPECT_TRUE(ifs.good()) << "Could not open " << f;
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()));
}

TEST(GapicGeneratorBaselineTest, StandaloneTest) {
  std::string const input_dir("../");
  std::string const output_dir("./");
  std::string const data_dir("./generator/testdata/");

  std::vector<std::string> descriptors = {
      input_dir +
          "com_google_gapic_generator_cpp/generator/testdata/"
          "library_proto-descriptor-set.proto.bin",
      input_dir +
          "com_google_googleapis/google/api/client_proto-descriptor-set.proto.bin",
      input_dir +
          "com_google_protobuf/descriptor_proto-descriptor-set.proto.bin"};
  std::string package = "google.example.library.v1";

  GapicGenerator generator;

  int res = StandaloneMain(descriptors, package, output_dir, &generator);
  EXPECT_EQ(0, res) << "StandaloneMain failed";

  std::vector<std::string> files_to_check{
      "google/example/library/v1/library_service.gapic.h",
      "google/example/library/v1/library_service.gapic.cc",
      "google/example/library/v1/library_service_stub.gapic.h",
      "google/example/library/v1/library_service_stub.gapic.cc",
  };

  for (auto const& file : files_to_check) {
    std::string const expected_file(data_dir + file + ".baseline");
    std::string const actual_file(output_dir + file);

    std::string const& expected_file_content = LoadContent(expected_file);
    std::string const& actual_file_content = LoadContent(actual_file);

    EXPECT_EQ(expected_file_content, actual_file_content)
        << "\nexpected_file: " << expected_file
        << "\nactual_file: " << actual_file;
  }
}

}  // namespace codegen
}  // namespace api
}  // namespace google

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

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <dirent.h>
#include <iostream>

#include "absl/strings/str_cat.h"

#include "src/google/protobuf/compiler/command_line_interface.h"
#include "src/google/protobuf/io/printer.h"
#include "src/google/protobuf/io/zero_copy_stream.h"

#include "googletest/include/gtest/gtest.h"

#include "generator/gapic_generator.h"

namespace {
namespace pb = google::protobuf;
namespace codegen = google::api::codegen;

inline std::string LoadContent(std::string f) {
  std::ifstream ifs(f);
  EXPECT_TRUE(ifs.good()) << "Could not open " << f;
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()));
}

TEST(CppGapicPluginTest, GapicPluginTest) {
  std::string workspace_dir = ".";
  std::string external_dir = workspace_dir + "/..";
  std::string test_dir = workspace_dir + "/generator";
  std::string test_data_dir = test_dir + "/testdata";
  std::string gapic_out_dir = workspace_dir;

  DIR* dirp = opendir(".");
  struct dirent* dp;
  while ((dp = readdir(dirp)) != NULL) {
    std::cout << dp->d_name << std::endl;
  }

  pb::compiler::CommandLineInterface cli;
  cli.SetInputsAreProtoPathRelative(true);

  codegen::GapicGenerator generator;
  cli.RegisterGenerator("--cpp_gapic_out", &generator, "");

  std::string workspace_proto_path = "-I" + workspace_dir;
  std::string annotations_proto_path =
      "-I" + external_dir + "/api_common_protos";
  std::string well_known_types_proto_path =
      "-I" + external_dir + "/com_google_protobuf";
  std::string cpp_out = "--cpp_gapic_out=" + gapic_out_dir;
  std::string library_proto = test_data_dir + "/library.proto";

  char const* argv[] = {"protoc",
                        workspace_proto_path.c_str(),
                        annotations_proto_path.c_str(),
                        well_known_types_proto_path.c_str(),
                        cpp_out.c_str(),
                        library_proto.c_str()};

  EXPECT_EQ(0, cli.Run(sizeof(argv) / sizeof(argv[0]), argv))
      << "cli.Run failed";

  std::vector<std::string> files_to_check{
      "google/example/library/v1/library_service.gapic.h",
      "google/example/library/v1/library_service.gapic.cc"};

  for (std::string const& f : files_to_check) {
    std::string actual_file = absl::StrCat(gapic_out_dir, "/", f);
    std::string expected_file =
        absl::StrCat(test_data_dir, "/", f, ".baseline");

    std::string actual_file_content = LoadContent(actual_file);
    std::string expected_file_content = LoadContent(expected_file);

    EXPECT_EQ(expected_file_content, actual_file_content);
  }
}

}  // namespace

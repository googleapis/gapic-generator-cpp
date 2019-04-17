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

#include <google/protobuf/compiler/plugin.h>
#include <gapic_generator.h>
#include <standalone.h>

/**
 * Entry point for C++ GAPIC generator.
 *
 * If at least one command line argument is provided, the standalone mode is
 * assumed. Otherwise the generator runs as a plugin (it expects input to be
 * received via stdin and outputs to stdout).
 */
int main(int argc, char** argv) {
  google::api::codegen::GapicGenerator generator;
  if (argc > 1) {
    return google::api::codegen::StandaloneMain(argc, argv, &generator);
  }
  // PluginMain immediately fails if argc > 1
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}

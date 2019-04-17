# Copyright 2019 Google Inc.  All rights reserved
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

workspace(name = "com_google_gapic_generator_cpp")

load(
    "//:repositories.bzl",
    "com_google_gapic_generator_cpp_gax_repositories",
    "com_google_gapic_generator_cpp_repositories",
)

com_google_gapic_generator_cpp_gax_repositories()
com_google_gapic_generator_cpp_repositories()

bind(
    name = "zlib",
    actual = "@net_zlib//:zlib",
)

# NOTE: this is a hack to allow grpc to build
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

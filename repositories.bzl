# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def com_google_gapic_generator_cpp_repositories():
    _maybe(
        http_archive,
        name = "absl",
        strip_prefix = "abseil-cpp-20181200",
        urls = ["https://github.com/abseil/abseil-cpp/archive/20181200.tar.gz"],
    )

    _maybe(
        http_archive,
        name = "com_google_protobuf",
        strip_prefix = "protobuf-3.7.1",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.7.1.tar.gz"],
    )

    _maybe(
        http_archive,
        name = "bazel_skylib",
        strip_prefix = "bazel-skylib-2169ae1c374aab4a09aa90e65efe1a3aad4e279b",
        urls = ["https://github.com/bazelbuild/bazel-skylib/archive/2169ae1c374aab4a09aa90e65efe1a3aad4e279b.tar.gz"],
    )

    _maybe(
        http_archive,
        name = "net_zlib",
        build_file = "@com_google_protobuf//:third_party/zlib.BUILD",
        strip_prefix = "zlib-1.2.11",
        urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
    )

    _maybe(
        native.bind,
        name = "zlib",
        actual = "@net_zlib//:zlib",
    )

    _maybe(
        http_archive,
        name = "com_google_googleapis",
        strip_prefix = "googleapis-c69355435cf6ae824a21f2bba31c69697733d3d2",
        urls = ["https://github.com/googleapis/googleapis/archive/c69355435cf6ae824a21f2bba31c69697733d3d2.tar.gz"],
    )

    _maybe(
        http_archive,
        name = "gtest",
        strip_prefix = "googletest-release-1.8.1",
        urls = ["https://github.com/google/googletest/archive/release-1.8.1.tar.gz"],
    )

def _maybe(repo_rule, name, **kwargs):
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

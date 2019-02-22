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


load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


def _protobuf():
    _maybe(
        http_archive,
        name = "com_google_protobuf",
        strip_prefix = "protobuf-master",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/master.zip"]
    )

    _maybe(
        http_archive,
        name = "bazel_skylib",
        sha256 = "bbccf674aa441c266df9894182d80de104cabd19be98be002f6d478aaa31574d",
        strip_prefix = "bazel-skylib-2169ae1c374aab4a09aa90e65efe1a3aad4e279b",
        urls = ["https://github.com/bazelbuild/bazel-skylib/archive/2169ae1c374aab4a09aa90e65efe1a3aad4e279b.tar.gz"]
    )

    _maybe(
        http_archive,
        name = "net_zlib",
        build_file = "@com_google_protobuf//:third_party/zlib.BUILD",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        urls = ["https://zlib.net/zlib-1.2.11.tar.gz"]
    )


def generator_dependencies():
    _maybe(
        http_archive,
        name = "absl",
        strip_prefix = "abseil-cpp-20181200",
        urls = ["https://github.com/abseil/abseil-cpp/archive/20181200.tar.gz"]
    )

    _protobuf()

    _maybe(
        http_archive,
        name = "api_common_protos",
        strip_prefix = "api-common-protos-input-contract",
        urls = ["https://github.com/michaelbausor/api-common-protos/archive/input-contract.zip"]
    )


def generated_client_dependencies():
    _maybe(
        http_archive,
        name = "com_github_grpc_grpc",
        strip_prefix = "grpc-master",
        urls = ["https://github.com/grpc/grpc/archive/master.zip"]
    )


def generator_test_dependencies():
    _maybe(
        http_archive,
        name = "gtest",
        strip_prefix = "googletest-master",
        urls = ["https://github.com/google/googletest/archive/master.zip"]
    )


def _maybe(repo_rule, name, **kwargs):
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

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


def com_google_gapic_generator_cpp_repositories():
    _maybe(
        http_archive,
        name = "absl",
        sha256 = "e2b53bfb685f5d4130b84c4f3050c81bf48c497614dc85d91dbd3ed9129bce6d",
        strip_prefix = "abseil-cpp-20181200",
        urls = ["https://github.com/abseil/abseil-cpp/archive/20181200.tar.gz"]
    )

    _maybe(
        http_archive,
        name = "com_google_protobuf",
        sha256 = "73fdad358857e120fd0fa19e071a96e15c0f23bb25f85d3f7009abfd4f264a2a",
        strip_prefix = "protobuf-3.6.1.3",
        urls = ["https://github.com/protocolbuffers/protobuf/archive/v3.6.1.3.tar.gz"]
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

    _maybe(
        http_archive,
        name = "api_common_protos",
        strip_prefix = "api-common-protos-87185dfffad4afa5a33a8c153f0e1ea53b4f85dc",
        urls = ["https://github.com/googleapis/api-common-protos/archive/87185dfffad4afa5a33a8c153f0e1ea53b4f85dc.tar.gz"]
    )

    _maybe(
        http_archive,
        name = "gtest",
        sha256 = "9bf1fe5182a604b4135edc1a425ae356c9ad15e9b23f9f12a02e80184c3a249c",
        strip_prefix = "googletest-release-1.8.1",
        urls = ["https://github.com/google/googletest/archive/release-1.8.1.tar.gz"]
    )


def com_google_gapic_generator_cpp_gax_repositories():
    _maybe(
        http_archive,
        name = "com_github_grpc_grpc",
        sha256 = "069a52a166382dd7b99bf8e7e805f6af40d797cfcee5f80e530ca3fc75fd06e2",
        strip_prefix = "grpc-1.18.0",
        urls = ["https://github.com/grpc/grpc/archive/v1.18.0.tar.gz"]
    )


def _maybe(repo_rule, name, **kwargs):
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

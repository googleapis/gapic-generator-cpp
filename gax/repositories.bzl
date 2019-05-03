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

def com_google_gapic_generator_cpp_gax_repositories():
#    _maybe(
#        http_archive,
#        name = "com_github_grpc_grpc",
#        strip_prefix = "grpc-1.18.0",
#        urls = ["https://github.com/grpc/grpc/archive/v1.18.0.tar.gz"],
#    )

    #TODO: update once cc_grpc_library update is merged
    _maybe(
        native.local_repository,
        name = "com_github_grpc_grpc",
        path = "/usr/local/google/home/vam/_/projects/github/vam-google/grpc",
    )

    _maybe(
        http_archive,
        name = "com_google_googleapis",
        strip_prefix = "googleapis-1a479920eb4f6c2bc4c8d8acd9280720540709e4",
        urls = ["https://github.com/googleapis/googleapis/archive/1a479920eb4f6c2bc4c8d8acd9280720540709e4.tar.gz"],
    )

def _maybe(repo_rule, name, **kwargs):
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

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

def com_google_gapic_generator_cpp_gax_repositories():
    _maybe(
        http_archive,
        name = "com_github_grpc_grpc",
        strip_prefix = "grpc-1.18.0",
        urls = ["https://github.com/grpc/grpc/archive/v1.18.0.tar.gz"]
    )


def _maybe(repo_rule, name, **kwargs):
    if name not in native.existing_rules():
        repo_rule(name = name, **kwargs)

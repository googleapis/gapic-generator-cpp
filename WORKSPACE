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

workspace(name = "com_google_gapic_generator_cpp")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load(
    "//gax:repositories.bzl",
    "com_google_gapic_generator_cpp_gax_repositories",
)
load(
    "//:repositories.bzl",
    "com_google_gapic_generator_cpp_repositories",
)

com_google_gapic_generator_cpp_gax_repositories()
com_google_gapic_generator_cpp_repositories()

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# WARNING: The java and go segments below are a hack to get
# C++ operations proto types to build.
# Tracked by https://github.com/googleapis/gapic-generator-cpp/issues/35

##############################################################################
# Java
##############################################################################

#
# gapic-generator
#
http_archive(
    name = "com_google_api_codegen",
    urls = ["https://github.com/googleapis/gapic-generator/archive/025ebdbb3d14609be938900a538418858b0ecfb7.zip"],
    strip_prefix = "gapic-generator-025ebdbb3d14609be938900a538418858b0ecfb7",
)

#
# java_gapic artifacts runtime dependencies (gax-java)
#
load("@com_google_api_codegen//rules_gapic/java:java_gapic_repositories.bzl", "java_gapic_repositories")

java_gapic_repositories()

load("@com_google_api_gax_java//:repository_rules.bzl", "com_google_api_gax_java_properties")

com_google_api_gax_java_properties(
    name = "com_google_api_gax_java_properties",
    file = "@com_google_api_gax_java//:dependencies.properties",
)

load("@com_google_api_gax_java//:repositories.bzl", "com_google_api_gax_java_repositories")

com_google_api_gax_java_repositories()
##############################################################################
# Go
##############################################################################

#
# rules_go (support Golang under bazel)
#
http_archive(
    name = "io_bazel_rules_go",
    strip_prefix = "rules_go-7d17d496a6b32f6a573c6c22e29c58204eddf3d4",
    urls = ["https://github.com/bazelbuild/rules_go/archive/7d17d496a6b32f6a573c6c22e29c58204eddf3d4.zip"],
)

load("@io_bazel_rules_go//go:def.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains()

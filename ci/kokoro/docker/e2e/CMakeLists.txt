# ~~~
# Copyright 2020 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ~~~

cmake_minimum_required(VERSION 3.5)

project(gapic-generator-e2e CXX)

# Configure the compiler options, we will be using C++11 features.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(googleapis REQUIRED)
find_package(gRPC REQUIRED)
find_package(gax REQUIRED)

add_library(google-cloud-cpp-spanner-gapic
        google/spanner/admin/instance/v1/instance_admin.gapic.cc
        google/spanner/admin/instance/v1/instance_admin.gapic.h
        google/spanner/admin/instance/v1/spanner_instance_admin.grpc.pb.cc
        google/spanner/admin/instance/v1/instance_admin_stub.gapic.h
        google/spanner/admin/instance/v1/spanner_instance_admin.grpc.pb.h
        google/spanner/admin/instance/v1/spanner_instance_admin.pb.h
        google/spanner/admin/instance/v1/spanner_instance_admin.pb.cc
        google/spanner/admin/instance/v1/instance_admin_stub.gapic.cc
)

target_include_directories(
        google-cloud-cpp-spanner-gapic
        PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

target_link_libraries(google-cloud-cpp-spanner-gapic PUBLIC
  gax
  googleapis-c++::api_annotations_protos
  googleapis-c++::longrunning_operations_protos
  googleapis-c++::rpc_status_protos
  googleapis-c++::iam_v1_iam_policy_protos
)

add_executable(gapic-generator-e2e
  spanner_e2e.cc
)

target_link_libraries(gapic-generator-e2e PUBLIC
	              google-cloud-cpp-spanner-gapic)

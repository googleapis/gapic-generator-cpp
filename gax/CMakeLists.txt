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

set(GAX_VERSION_MAJOR 0)
set(GAX_VERSION_MINOR 1)
set(GAX_VERSION_PATCH 0)

string(CONCAT GAX_VERSION
              "${GAX_VERSION_MAJOR}"
              "."
              "${GAX_VERSION_MINOR}"
              "."
              "${GAX_VERSION_PATCH}")

include(GapicGeneratorCommon)

find_package(googleapis REQUIRED)
find_package(gRPC REQUIRED)


add_library(gax
    # cmake-format: sortable
    backoff_policy.cc
    backoff_policy.h
    call_context.cc
    call_context.h
    internal/gtest_prod.h
    internal/invoke_result.h
    operation.h
    operations_client.cc
    operations_client.h
    operations_stub.cc
    operations_stub.h
    pagination.h
    retry_loop.h
    retry_policy.h
    status.cc
    status.h
    status_or.h)

target_include_directories(
    gax
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

set_target_properties(
    gax
    PROPERTIES VERSION ${GAX_VERSION} SOVERSION
    ${GAX_VERSION_MAJOR})

target_link_libraries(gax PUBLIC googleapis-c++::longrunning_operations_protos)

# Export the CMake targets to make it easy to create configuration files.
install(EXPORT gax-targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gax")

install(
    TARGETS gax
    EXPORT gax-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT gax_runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT gax_runtime
            NAMELINK_SKIP
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT gax_development)
# With CMake-3.12 and higher we could avoid this separate command (and the
# duplication).
install(
    TARGETS gax
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT gax_development
            NAMELINK_ONLY
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT gax_development)

gax_install_headers(gax include/gax)

if (BUILD_TESTING)
    set(gax_unit_tests
        # cmake-format: sortable
        backoff_policy_test.cc
        operations_stub_test.cc
        operation_test.cc
        pagination_test.cc
        retry_loop_test.cc
        retry_policy_test.cc
        status_or_test.cc
        status_test.cc
    )
    foreach (fname ${gax_unit_tests})
        string(REPLACE "/" "_" target ${fname})
        string(REPLACE ".cc" "" target ${target})
        add_executable(${target} ${fname})
        target_link_libraries(
            ${target}
            PRIVATE gax
                    GTest::gmock_main GTest::gmock GTest::gtest)
        if (MSVC)
            target_compile_options(${target} PRIVATE "/bigobj")
        endif ()
        add_test(NAME ${target} COMMAND ${target})
    endforeach ()
endif()

# Create and install the CMake configuration files.
configure_file("config.cmake.in" "gax-config.cmake" @ONLY)
configure_file("config-version.cmake.in"
               "gax-config-version.cmake" @ONLY)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/gax-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/gax-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gax")

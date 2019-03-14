#!/usr/bin/env bash
#
# Copyright 2018 Google LLC
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

set -eu

echo "================================================================"
echo "Running Bazel unit tests $(date)."
echo "================================================================"

echo "Running build and tests"
cd "$(dirname $0)/../../.."
readonly PROJECT_ROOT="${PWD}"

echo "================================================================"
echo "Update or Install Bazel $(date)."
echo "================================================================"
"${PROJECT_ROOT}/ci/install-bazel.sh" linux

readonly BAZEL_BIN="$HOME/bin/bazel"
echo "Using Bazel in ${BAZEL_BIN}"

# Kokoro does guarantee that g++-4.9 will be installed, but the default compiler
# might be g++-4.8. Set the compiler version explicitly:
export CC=/usr/bin/gcc-4.9
export CXX=/usr/bin/g++-4.9

echo "================================================================"
echo "Compiling and running unit tests $(date)"
echo "================================================================"
"${BAZEL_BIN}" test \
    --test_output=errors \
    --verbose_failures=true \
    --keep_going \
    -- //...:all

echo "================================================================"
echo "Build completed $(date)"
echo "================================================================"

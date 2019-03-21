#!/usr/bin/env bash
#
# Copyright 2019 Google Inc.
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

# This script assumes it is running the top-level gapic-generator-cpp directory.

readonly BINDIR="$(dirname $0)"

function check_style {
  find $1 -name '*.h' -print0 \
    | xargs -0 awk -f ${BINDIR}/check-include-guards.gawk

  find $1 \( -name '*.cc' -o -name '*.h' \) -print0 \
    | xargs -0 ${CLANG_FORMAT_BIN} -format -i
}

check_style gax
check_style generator

# Report any differences created by running clang-format.
git diff --ignore-submodules=all --color --exit-code .

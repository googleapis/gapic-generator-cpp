#!/usr/bin/env bash
#
# Copyright 2019 Google LLC
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

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <linux|osx>"
fi

if [[ "$1" = "linux" ]]; then
  # OK
  echo "Installing clang-format on linux platform"
else
  echo "Unsupported platform for clang-format: $1"
  exit 1
fi

readonly CLANG_FORMAT_VERSION=3.9
sudo apt-get install "clang-format-${CLANG_FORMAT_VERSION}"
export CLANG_FORMAT_BIN="$(which clang-format-${CLANG_FORMAT_VERSION})"


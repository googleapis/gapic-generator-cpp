#!/usr/bin/env bash
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

set -eu

if [[ -z "${PROJECT_ROOT+x}" ]]; then
  PROJECT_ROOT="$(cd "$(dirname "$0")/../../.."; pwd)"
  readonly PROJECT_ROOT
fi

echo "================================================================"
echo "Change working directory to project root $(date)."
cd "${PROJECT_ROOT}"

if [[ -n "${PROJECT_ID:-}" ]]; then
  DOCKER_IMAGE_PREFIX="gcr.io/${PROJECT_ID}/gapic-generator"
else
  # We want a prefix that works when running interactively, so it must be a
  # (syntactically) valid project id, this works.
  DOCKER_IMAGE_PREFIX="gcr.io/cloud-cpp-reserved/gapic-generator"
fi
readonly DOCKER_IMAGE_PREFIX

# First build the Docker image
readonly IMAGE="${DOCKER_IMAGE_PREFIX}/e2e-ubuntu-18.04"

echo "================================================================"
echo "Building a Docker image $(date)."
docker build -f "${PROJECT_ROOT}/ci/kokoro/docker/e2e/Dockerfile" . \
  -t "${IMAGE}"

if [[ -n "${GOOGLE_APPLICATION_CREDENTIALS:-}" \
  && -n "${TEST_PROJECT_ID:-}" ]]; then
  echo "================================================================"
  echo "Executing the binary $(date)."
  # Run the binary
  docker run --rm \
    -v "${GOOGLE_APPLICATION_CREDENTIALS}:/credentials.json" \
    -e "GOOGLE_APPLICATION_CREDENTIALS=/credentials.json" \
    -e "TEST_PROJECT_ID=${TEST_PROJECT_ID}" \
    "${IMAGE}" \
    bash -c "cd /build/gapic-generator-cpp/ci/kokoro/docker/e2e && cmake-out/gapic-generator-e2e"
else
  echo "================================================================"
  echo -n "Set GOOGLE_APPLICATION_CREDENTIALS and TEST_PROJECT_ID"
  echo " environment variables to run the binary."
  echo "Skip executing binary $(date)."
fi

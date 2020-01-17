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

FROM ubuntu:18.04
ARG NCPU=4

# Dependencies
RUN \
  apt-get update && \
  apt-get -y install \
          autoconf \
          build-essential \
          clang-tidy \
          cmake \
          curl \
          gcc \
          git-core \
          golang \
          g++ \
          libtool \
          libssl-dev \
          make \
          pkg-config \
          wget \
          vim \
          zip \
          zlib1g-dev

WORKDIR /build

# Install c-ares
RUN \
  wget -q https://github.com/c-ares/c-ares/archive/cares-1_15_0.tar.gz && \
  tar -xf cares-1_15_0.tar.gz && \
  cd c-ares-cares-1_15_0 && \
  cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -H. -Bcmake-out && \
  cmake --build cmake-out -- -j ${NCPU:-4} && \
  cmake --build cmake-out --target install && \
  ldconfig

# Install protobuf
RUN \
  wget -q https://github.com/google/protobuf/archive/v3.11.2.tar.gz && \
  tar -xf v3.11.2.tar.gz && \
  cd protobuf-3.11.2/cmake && \
  cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=yes \
        -Dprotobuf_BUILD_TESTS=OFF \
        -H. -Bcmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
    cmake --build cmake-out --target install -- && \
    ldconfig

# Install gRPC
RUN \
  wget -q https://github.com/grpc/grpc/archive/v1.26.0.tar.gz && \
  tar -xf v1.26.0.tar.gz && \
  cd grpc-1.26.0 && \
  cmake \
    -DgRPC_CARES_PROVIDER=package \
    -DgRPC_PROTOBUF_PROVIDER=package \
    -DgRPC_SSL_PROVIDER=package \
    -DgRPC_ZLIB_PROVIDER=package \
    -DgRPC_INSTALL=on \
    -H. -Bcmake-out && \
  cd cmake-out && \
  CONFIG=dbg make -j ${NCPU:-4} && \
  CONFIG=dbg make install

# Install abseil
RUN \
  wget -q https://github.com/abseil/abseil-cpp/archive/20190808.tar.gz && \
  tar -xf 20190808.tar.gz && \
  cd abseil-cpp-20190808 && \
  cmake \
        -DBUILD_SHARED_LIBS=yes \
        -H. -Bcmake-out && \
  cmake --build cmake-out -- -j ${NCPU:-4} && \
  cmake --build cmake-out --target install && \
  ldconfig

# Install cpp-cmakefiles
RUN \
  wget -q \
       https://github.com/googleapis/cpp-cmakefiles/archive/v0.2.1.tar.gz && \
  tar zxvf v0.2.1.tar.gz && \
  cd cpp-cmakefiles-0.2.1 && \
  cmake \
        -DBUILD_SHARED_LIBS=yes \
        -H. -Bcmake-out && \
  cmake --build cmake-out -- -j ${NCPU:-4} && \
  cmake --build cmake-out --target install && \
  ldconfig

# Install GTest
RUN \
    wget -q \
         https://github.com/google/googletest/archive/release-1.10.0.tar.gz && \
    tar -xf release-1.10.0.tar.gz && \
    cd googletest-release-1.10.0 && \
    cmake \
          -DCMAKE_BUILD_TYPE="Release" \
          -DBUILD_SHARED_LIBS=yes \
          -H. -Bcmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
    cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
    ldconfig

# Install gax
COPY . /build/gapic-generator-cpp/
RUN \
    cd /build/gapic-generator-cpp && \
    cmake \
          -DBUILD_SHARED_LIBS=on \
          -H. -Bcmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4} && \
    (cd cmake-out; ctest) && \
    cmake --build cmake-out --target install && \
    ldconfig

# Install bazel
RUN /build/gapic-generator-cpp/ci/install-bazel.sh linux

# Build gapic plugin
RUN \
    cd /build/gapic-generator-cpp && \
    /root/bin/bazel build //...

# Use the newer googleapis repo for default_host option
RUN \
    git clone https://github.com/googleapis/googleapis

# Run protoc with all the plugin
ENV GOOGLEAPIS_PATH=/build/googleapis
RUN \
protoc --proto_path=${GOOGLEAPIS_PATH} \
           --cpp_out=/build/gapic-generator-cpp/ci/kokoro/docker/e2e \
           --grpc_out=/build/gapic-generator-cpp/ci/kokoro/docker/e2e \
           --cpp_gapic_out=/build/gapic-generator-cpp/ci/kokoro/docker/e2e \
           --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin \
           --plugin=protoc-gen-cpp_gapic=/build/gapic-generator-cpp/bazel-bin/generator/protoc-gen-cpp_gapic \
           ${GOOGLEAPIS_PATH}/google/spanner/admin/instance/v1/spanner_instance_admin.proto

# Compile the generated files
RUN \
    cd /build/gapic-generator-cpp/ci/kokoro/docker/e2e && \
    cmake -H. -Bcmake-out && \
    cmake --build cmake-out -- -j ${NCPU:-4}

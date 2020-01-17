// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <grpcpp/grpcpp.h>
#include "google/spanner/admin/instance/v1/instance_admin.gapic.h"

using ::google::spanner::admin::instance::v1::ListInstanceConfigsRequest;

int main() {
  InstanceAdmin client(CreateInstanceAdminStub());
  ListInstanceConfigsRequest request;
  if (const char* project_id = std::getenv("GOOGLE_CLOUD_PROJECT")) {
    std::string proj = "projects/";
    request.set_parent(proj + project_id);
  } else {
    throw std::runtime_error("Please set GOOGLE_CLOUD_PROJECT env");
  }
  auto result = client.ListInstanceConfigs(request);
  if (!result) {
    throw std::runtime_error(result.status().message());
  }
  auto& configs = *result->mutable_instance_configs();
  int count = 0;
  for (auto const& config : configs) {
    ++count;
    std::cout << "Instance config [" << count << "]:\n"
	      << config.DebugString() << "\n";
  }
}

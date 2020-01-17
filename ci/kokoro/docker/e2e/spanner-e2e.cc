#include <grpc++/grpc++.h>
#include "google/spanner/admin/instance/v1/instance_admin.gapic.h"

using ::google::spanner::admin::instance::v1::ListInstanceConfigsRequest;

int main() {
  auto creds = grpc::GoogleDefaultCredentials();
  auto stub = CreateInstanceAdminStub(creds);
  InstanceAdmin client(std::move(stub));
  ListInstanceConfigsRequest request;
  if (const char* project_id = std::getenv("TEST_PROJECT_ID")) {
    std::string proj = "projects/";
    request.set_parent(proj + project_id);
  } else {
    throw std::runtime_error("Please set TEST_PROJECT_ID env");
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

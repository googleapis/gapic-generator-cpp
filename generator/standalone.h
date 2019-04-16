#ifndef GENERATOR_STANDALONE_H_
#define GENERATOR_STANDALONE_H_

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/plugin.pb.h>

namespace google {
namespace api {
namespace codegen {

int StandaloneMain(int argc,
                   const char* const argv[],
                   google::protobuf::compiler::CodeGenerator* generator);

int StandaloneMain(const std::vector<std::string>& descriptors,
                   const std::string& package,
                   const std::string& output,
                   google::protobuf::compiler::CodeGenerator* generator);

}  // namespace google
}  // namespace api
}  // namespace codegen
#endif  // GENERATOR_STANDALONE_H_

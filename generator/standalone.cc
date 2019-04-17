#include <fstream>
#include <standalone.h>

#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/stubs/io_win32.h>
#include <gapic_generator.h>

#include "absl/strings/str_split.h"

namespace google {
namespace api {
namespace codegen {

namespace pb = google::protobuf;

// GAPIC Standalone expects proto package as an argument. It defines the API
// for which the GAPIC code must be generated for (i.e. to distinguish between
// common proto descriptors and the actual API descriptors). Protobuf compiler
// and the corresponding google::protobuf::CommandLineInterface expect the list
// of the API-specific .proto files instead.
//
// This function extracts the relevant file names (those, which match the
// package) from the set of descriptors.
bool ExtractFileNames(std::vector<std::string> const& desc_sets,
                      std::vector<std::string> const& packages,
                      std::vector<std::string>* filenames,
                      std::string* error_msg) {
  for (auto const& desc_set : desc_sets) {
    if (desc_set.empty()) {
      continue;
    }

    std::ifstream fstr(desc_set);
    if (!fstr) {
      *error_msg = "Could not open file " + desc_set;
      return false;
    }

    pb::FileDescriptorSet bin_desc_set;
    bool parse_result = bin_desc_set.ParseFromIstream(&fstr);

    fstr.close();
    if (!parse_result) {
      return false;
    }

    for (int i = 0; i < bin_desc_set.file_size(); i++) {
      auto const& fl = bin_desc_set.file(i);
      auto iter = std::find(packages.begin(), packages.end(), fl.package());
      if (iter != packages.end()) {
        filenames->emplace_back(fl.name());
      }
    }
  }

  return true;
}

// Converts arguments from what is required by GAPIC generator standalone mode
// specification (--descriptor, --package, --output) to what is understood by
// google::protobuf::CommandLineInterface. CommandLineInterface seems like the
// preferred way of writing GAPIC generator in C++:
// - it is what is used by protoc main() itself;
// - it does not spawn a new subprocess if the generator was explicitly
//   registered by CommandLineInterface::RegisterGenerator(), so it is
//   performance efficient and good for debugging (runs in same process);
// - it handles platform-specific file I/O (including writing into zip archive);
//   this is especially useful for folder operations, since standard library
//   below C++17 does not have folder I/O abstraction whatsoever.
//
bool ConvertCommandLineArgs(int argc, char const* const argv[],
                            std::vector<std::string>& args,
                            std::string* error_msg) {
  // GAPIC Generator Standalone arguments
  std::string const desc_arg("--descriptor");
  std::string const output_arg("--output");
  std::string const package_arg("--package");

  std::string const desc_set_in_arg("--descriptor_set_in=");

  std::vector<std::string> desc_set_in;
  std::vector<std::string> packages;

  for (int i = 0; i < argc; i++) {
    std::vector<std::string> arg =
        absl::StrSplit(argv[i], absl::MaxSplits('=', 1));
    if (arg.size() <= 1) {
      args.emplace_back(argv[i]);
      continue;
    }

    std::string const& arg_name = arg[0];
    std::string const& arg_val = arg[1];

    if (arg_name == desc_arg || arg_name == desc_set_in_arg) {
      args.emplace_back(desc_set_in_arg + arg_val);
      std::vector<std::string> const& spl =
          absl::StrSplit(arg_val, absl::ByAnyChar(":;"));
      std::move(spl.begin(), spl.end(), std::back_inserter(desc_set_in));
    } else if (arg_name == output_arg) {
      args.emplace_back("--cpp_gapic_out=" + arg_val);
    } else if (arg_name == package_arg) {
      std::vector<std::string> const& spl =
          absl::StrSplit(arg_val, absl::ByAnyChar(":;"));
      std::move(spl.begin(), spl.end(), std::back_inserter(packages));
    } else {
      args.emplace_back(argv[i]);
    }
  }

  if (!desc_set_in.empty()) {
    std::vector<std::string> file_names;
    if (!ExtractFileNames(desc_set_in, packages, &file_names, error_msg)) {
      return false;
    }
    args.insert(args.end(), std::make_move_iterator(file_names.begin()),
                std::make_move_iterator(file_names.end()));
  }

  return true;
}

int StandaloneMain(int argc, char const* const argv[],
                   google::protobuf::compiler::CodeGenerator* generator) {
  std::string error_msg;
  std::vector<std::string> args;
  ConvertCommandLineArgs(argc, argv, args, &error_msg);

  pb::compiler::CommandLineInterface cli;
  cli.RegisterGenerator("--cpp_gapic_out", generator, "GAPIC C++ Generator");

  std::vector<char const*> c_args;
  c_args.reserve(args.size());
  for (auto const& arg : args) {
    c_args.push_back(arg.c_str());
  }

  return cli.Run((int)args.size(), c_args.data());
}

int StandaloneMain(std::vector<std::string> const& descriptors,
                   std::string const& package, std::string const& output,
                   google::protobuf::compiler::CodeGenerator* generator) {
  std::string desc_set_in_arg("--descriptor=");
  for (auto const& desc_set : descriptors) {
    desc_set_in_arg += desc_set + ":";
  }
  std::string package_arg("--package=" + package);
  std::string output_arg("--output=" + output);

  std::vector<char const*> c_args = {"<ignored>", desc_set_in_arg.c_str(),
                                     package_arg.c_str(), output_arg.c_str()};

  return StandaloneMain((int)c_args.size(), c_args.data(), generator);
};

}  // namespace codegen
}  // namespace api
}  // namespace google

#include <fstream>
#include <standalone.h>

#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/stubs/io_win32.h>
#include <gapic_generator.h>

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
bool ExtractFileNames(const std::vector<std::string>& desc_sets,
                      const std::vector<std::string>& packages,
                      std::vector<std::string>& filenames,
                      std::string* error_msg) {
  for (const std::string& desc_set : desc_sets) {
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
      const auto& fl = bin_desc_set.file(i);
      auto iter = std::find(packages.begin(), packages.end(), fl.package());
      if (iter != packages.end()) {
        filenames.emplace_back(fl.name());
      }
    }
  }

  return true;
}

void ParseDelimitedArg(const std::string& arg,
                       std::vector<std::string>& tokens) {
  // Linux will use ':' and Windows will use ';' as the the delimiter.
  // Accept both as valid delimiters (not allowed to be used simultaneously).
  const char* delimiters = ":;";
  size_t p0 = 0;
  for (const char* d = delimiters; *d != '\0' && tokens.empty(); d++) {
    p0 = 0;
    for (size_t p = arg.find(*d, 0); p != std::string::npos;
         p0 = p + 1, p = arg.find(*d, p0)) {
      tokens.emplace_back(arg.substr(p0, p - p0));
    }
  }
  if (arg.size() > p0) {
    tokens.emplace_back(arg.substr(p0, arg.size() - p0));
  }
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
bool ConvertCommandLineArgs(int argc, const char* const argv[],
                            std::vector<std::string>& args,
                            std::string* error_msg) {
  // GAPIC Generator Standalone arguments
  const std::string desc_arg("--descriptor");
  const std::string output_arg("--output");
  const std::string package_arg("--package");

  const std::string desc_set_in_arg("--descriptor_set_in=");

  std::vector<std::string> desc_set_in;
  std::vector<std::string> packages;

  for (int i = 0; i < argc; i++) {
    std::string arg(argv[i]);
    auto pos = arg.find('=', 0);
    if (pos == std::string::npos) {
      args.emplace_back(arg);
      continue;
    }

    const std::string& arg_name = arg.substr(0, pos);
    const std::string& arg_val = arg.substr(pos + 1, arg.size() - pos);

    if (arg_name == desc_arg || arg_name == desc_set_in_arg) {
      args.emplace_back(desc_set_in_arg + arg_val);
      ParseDelimitedArg(arg_val, desc_set_in);
    } else if (arg_name == output_arg) {
      args.emplace_back("--cpp_gapic_out=" + arg_val);
    } else if (arg_name == package_arg) {
      ParseDelimitedArg(arg_val, packages);
    } else {
      args.emplace_back(arg);
    }
  }

  if (!desc_set_in.empty()) {
    std::vector<std::string> file_names;
    if (!ExtractFileNames(desc_set_in, packages, file_names, error_msg)) {
      return false;
    }
    args.insert(args.end(), std::make_move_iterator(file_names.begin()),
                std::make_move_iterator(file_names.end()));
  }

  return true;
}

int StandaloneMain(int argc, const char* const argv[],
                   google::protobuf::compiler::CodeGenerator* generator) {
  std::string error_msg;
  std::vector<std::string> args;
  ConvertCommandLineArgs(argc, argv, args, &error_msg);

  pb::compiler::CommandLineInterface cli;
  cli.RegisterGenerator("--cpp_gapic_out", generator, "GAPIC C++ Generator");

  std::vector<const char*> c_args;
  c_args.reserve(args.size());
  for (const std::string& arg : args) {
    c_args.push_back(arg.c_str());
  }

  return cli.Run((int)args.size(), c_args.data());
}

int StandaloneMain(const std::vector<std::string>& descriptors,
                   const std::string& package, const std::string& output,
                   google::protobuf::compiler::CodeGenerator* generator) {
  std::string desc_set_in_arg("--descriptor=");
  for (const std::string& desc_set : descriptors) {
    desc_set_in_arg += desc_set + ":";
  }
  std::string package_arg("--package=" + package);
  std::string output_arg("--output=" + output);

  std::vector<const char*> c_args = {"<ignored>", desc_set_in_arg.c_str(),
                                     package_arg.c_str(), output_arg.c_str()};

  return StandaloneMain((int)c_args.size(), c_args.data(), generator);
};

}  // namespace codegen
}  // namespace api
}  // namespace google

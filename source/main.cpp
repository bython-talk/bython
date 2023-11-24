#include <iostream>
#include <memory>
#include <string>

#include <bython/executors/jit.hpp>
#include <llvm/Support/CommandLine.h>

enum class compilation_mode
{
  parse_only,
  type_check,
  full,
};

auto main(int argc, char* argv[]) -> int
{

  namespace cl = llvm::cl;

  auto jit_category = cl::OptionCategory {
      "Driver Options", "Options for controlling Bython's driver"};

  auto inpath = cl::opt<std::string>("inpath",
                                     cl::desc("File to just-in-time compile and execute"),
                                     cl::value_desc("filepath"),
                                     cl::Required,
                                     cl::cat(jit_category));

  auto debug_values =
      cl::values(clEnumValN(compilation_mode::parse_only, "parse", "Disable optimisations, enable debugging"),
                 clEnumValN(compilation_mode::type_check, "tcheck", "Disable both optimisations and debugging"),
                 clEnumValN(compilation_mode::full, "full", "Enable optimisations, disable debugging"));
  auto debug = cl::opt<compilation_mode>("m",
                                  cl::desc("Choose compilation mode"),
                                  debug_values,
                                  cl::value_desc("comp-mode"),
                                  cl::init(compilation_mode::full),
                                  cl::cat(jit_category));

  cl::HideUnrelatedOptions(jit_category);
  cl::ParseCommandLineOptions(argc, argv, "bython-jit");

  auto jit = bython::executor::jit_compiler {};
  return jit.execute(inpath.getValue());
}

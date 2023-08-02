#include <iostream>
#include <memory>
#include <string>

#include <bython/executors/jit.hpp>

#include <llvm/Support/CommandLine.h>

enum class opt_level
{
  debug,
  off,
  on
};

auto main(int argc, char* argv[]) -> int
{
  namespace cl = llvm::cl;

  auto inpath =
      cl::opt<std::string>("inpath",
                           cl::desc("File to just-in-time compile and execute"),
                           cl::value_desc("filepath"),
                           cl::Required);

  auto debug_values = cl::values(
      clEnumValN(
          opt_level::debug, "g", "Disable optimisations, enable debugging"),
      clEnumValN(
          opt_level::off, "0", "Disable both optimisations and debugging"),
      clEnumValN(
          opt_level::on, "fast", "Enable optimisations, disable debugging"));
  auto debug = cl::opt<opt_level>("O",
                                  cl::desc("Choose optimisation level"),
                                  debug_values,
                                  cl::value_desc("opt-level"),
                                  cl::init(opt_level::off));

  cl::ParseCommandLineOptions(argc, argv, "bython-jit");

  auto jit = bython::executor::jit_compiler{};
  return jit.execute(inpath.getValue());
}

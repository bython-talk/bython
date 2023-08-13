#include <iostream>
#include <memory>
#include <string>

#include <bython/executors/interpreter.hpp>
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

  auto repl_category = cl::OptionCategory {
      "REPL Options", "Options for controlling the REPL implementation of Bython"};

  auto debug_values =
      cl::values(clEnumValN(opt_level::debug, "g", "Disable optimisations, enable debugging"),
                 clEnumValN(opt_level::off, "0", "Disable both optimisations and debugging"),
                 clEnumValN(opt_level::on, "fast", "Enable optimisations, disable debugging"));
  auto debug = cl::opt<opt_level>("O",
                                  cl::desc("Choose optimisation level"),
                                  debug_values,
                                  cl::value_desc("opt-level"),
                                  cl::init(opt_level::off),
                                  cl::cat(repl_category));

  cl::HideUnrelatedOptions(repl_category);
  cl::ParseCommandLineOptions(argc, argv, "bython-interpreter");

  auto jit = bython::executor::interpreter {};
  jit.begin();

  return EXIT_SUCCESS;
}

#include <iostream>
#include <memory>
#include <string>

#include <llvm/Support/CommandLine.h>

enum class opt_level {
  debug, off, on
};

auto main(int argc, char* argv[]) -> int
{
  namespace cl = llvm::cl;

  auto debug = cl::opt<opt_level>(
    cl::desc("Choose optimisation level"),
    cl::values(
      clEnumValN(opt_level::debug, "g", "Disable optimisations, enable debugging"),
      clEnumValN(opt_level::off, "O0", "Disable both optimisations and debugging"),
      clEnumValN(opt_level::on, "Ofast", "Enable all optimisations, disable debugging")
    )
  );

  cl::ParseCommandLineOptions(argc, argv, "bython-repl");
}

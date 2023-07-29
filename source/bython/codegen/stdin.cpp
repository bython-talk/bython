#include "stdin.hpp"

namespace bython::codegen
{

auto stdin::into_module() -> std::unique_ptr<llvm::Module>
{
  return nullptr;
}

auto stdin::execute() const -> void {}

}  // namespace bython::codegen

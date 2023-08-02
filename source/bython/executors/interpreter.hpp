#pragma once

#include <memory>
#include <string_view>

#include <bython/ast/bases.hpp>

namespace bython::codegen
{
struct interpreter
{
  interpreter();
  ~interpreter();

  interpreter(interpreter const&) = delete;
  auto operator=(interpreter const&) noexcept -> interpreter& = delete;

  interpreter(interpreter&&);
  auto operator=(interpreter&&) noexcept -> interpreter&;

  auto begin() -> void;

private:
  struct interpreter_pimpl;
  std::unique_ptr<interpreter_pimpl> impl;
};

}  // namespace bython::codegen
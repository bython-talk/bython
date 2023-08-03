#pragma once

#include <cinttypes>
#include <string>

namespace bython::codegen
{

struct builtin_metadata
{
  std::string name;
  std::uint64_t procedure_addr;
};

enum class builtin_tag
{
  put_i64,
  putln_i64
};

auto builtin(builtin_tag tag) -> builtin_metadata;

}  // namespace bython::codegen

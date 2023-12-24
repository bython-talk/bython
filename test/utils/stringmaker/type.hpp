#pragma once

#include <bython/type_system/builtin.hpp>
#include <catch2/catch_tostring.hpp>

#include "type.hpp"

namespace Catch
{
template<>
struct StringMaker<bython::type_system::type*>
{
  static auto convert(bython::type_system::type* const& value) -> std::string;
};
}  // namespace Catch
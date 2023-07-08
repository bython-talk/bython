#pragma once

#include <lexy/dsl/ascii.hpp>

namespace bython::grammar
{
template<typename T>
struct top_level
{
  static constexpr auto whitespace = lexy::dsl::ascii::space;

  static constexpr auto rule = T::rule;
  static constexpr auto value = T::value;
};
}  // namespace bython::grammar
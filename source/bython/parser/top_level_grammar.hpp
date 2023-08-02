#pragma once

#include <lexy/dsl/ascii.hpp>

namespace bython::grammar
{
template<typename T>
struct top_level
{
  static constexpr auto whitespace = lexy::dsl::ascii::space
      | LEXY_LIT("#") >> dsl::until(dsl::newline);

  static constexpr auto rule = T::rule + lexy::dsl::whitespace(whitespace) + lexy::dsl::eof;
  static constexpr auto value = T::value;
};
}  // namespace bython::grammar
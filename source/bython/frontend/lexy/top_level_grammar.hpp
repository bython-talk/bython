#pragma once

#include <lexy/dsl.hpp>

namespace bython::grammar
{
namespace dsl = lexy::dsl;

template<typename T>
struct top_level
{
  static constexpr auto whitespace = dsl::ascii::space | LEXY_LIT("#") >> dsl::until(dsl::newline)
      | dsl::backslash >> dsl::newline;

  static constexpr auto rule = T::rule + dsl::whitespace(whitespace) + dsl::eof;
  static constexpr auto value = T::value;
};

template<typename T>
struct repl_top_level
{
  static constexpr auto whitespace = dsl::ascii::blank | LEXY_LIT("#") >> dsl::until(dsl::newline)
      | dsl::backslash >> dsl::newline;

  static constexpr auto rule = []
  {
    // We can't use `dsl::eol` as our terminator directly,
    // since that would try and skip whitespace, which requests more input on the REPL.
    auto at_eol = dsl::peek(dsl::eol);
    return dsl::terminator(at_eol)(T::rule);
  }();

  // static constexpr auto rule = T::rule + lexy::dsl::whitespace(whitespace) + lexy::dsl::eof;
  static constexpr auto value = T::value;
};
}  // namespace bython::grammar
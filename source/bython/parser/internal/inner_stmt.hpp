#pragma once

#include <memory>
#include <string>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include "bython/ast/statement.hpp"
#include "expression.hpp"
#include "primitives.hpp"

namespace bython::grammar
{
namespace dsl = lexy::dsl;

struct assignment
{
  static constexpr auto rule = []
  {
    return keyword::variable_ + dsl::whitespace(dsl::ascii::space)
        + dsl::p<grammar::symbol_identifier>
        + LEXY_LIT("=") + dsl::p<expression>;
  }();

  static constexpr auto value = lexy::construct<ast::assignment>
      | lexy::new_<ast::assignment, std::unique_ptr<ast::statement>>;
};

struct inner_stmt
{
  static constexpr auto rule = []
  { return dsl::peek(keyword::variable_) >> dsl::p<assignment>; }();

  static constexpr auto value = lexy::forward<std::unique_ptr<ast::statement>>;
};
};  // namespace bython::grammar
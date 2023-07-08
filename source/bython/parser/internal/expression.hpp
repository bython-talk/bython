#pragma once

#include <memory>
#include <string>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include "bython/ast/expression.hpp"
#include "primitives.hpp"

namespace bython::grammar
{

struct expression;

struct call
{
  struct argument_list
  {
    static constexpr auto rule = []
    {
      return dsl::round_bracketed.opt_list(dsl::recurse<expression>,
                                           dsl::trailing_sep(dsl::comma));
    }();

    static constexpr auto value =
        lexy::as_list<std::vector<std::unique_ptr<ast::expression>>>;
  };

  static constexpr auto rule = []
  { return dsl::p<symbol_identifier> + dsl::p<argument_list>; }();

  static constexpr auto value = lexy::construct<ast::call>
      | lexy::new_<ast::call, std::unique_ptr<ast::expression>>;
};

struct variable
{
  static constexpr auto rule = [] { return dsl::p<symbol_identifier>; }();

  static constexpr auto value = lexy::construct<ast::variable>
      | lexy::new_<ast::variable, std::unique_ptr<ast::expression>>;
};

struct expression
{
  static constexpr auto rule = []
  {
    return dsl::lookahead(dsl::lit_c<'('>, dsl::newline) >> dsl::p<call>
        | dsl::else_ >> dsl::p<variable>;
  }();

  static constexpr auto value = lexy::forward<std::unique_ptr<ast::expression>>;
};
}  // namespace bython::grammar
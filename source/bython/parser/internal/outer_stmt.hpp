#pragma once

#include <string>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include "bython/ast/statement.hpp"
#include "inner_stmt.hpp"
#include "primitives.hpp"

namespace bython::grammar
{
namespace dsl = lexy::dsl;

struct compound_body
{
  static constexpr auto rule = []
  {
    auto terminator = dsl::terminator(dsl::semicolon).limit(dsl::lit_c<'}'>);
    return dsl::curly_bracketed.opt_list(terminator(dsl::p<inner_stmt>));
  }();

  static constexpr auto value = lexy::as_list<ast::statements>;
};

struct function_def
{
  struct parameter
  {
    static constexpr auto rule = [] { return dsl::p<symbol_identifier>; }();
    static constexpr auto value = lexy::construct<ast::parameter>;
  };

  struct parameters
  {
    static constexpr auto rule = []
    {
      return dsl::round_bracketed.opt_list(dsl::p<parameter>,
                                           dsl::trailing_sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_list<ast::parameters>;
  };

  static constexpr auto rule = []
  {
    return keyword::funcdef_ + dsl::whitespace(dsl::ascii::space)
        + dsl::p<grammar::symbol_identifier> + dsl::p<parameters>
        + dsl::p<compound_body>;
  }();

  static constexpr auto value = lexy::construct<ast::function_def>;
};

struct type_def
{
  struct body
  {
    static constexpr auto rule = []
    {
      return dsl::curly_bracketed.opt_list(dsl::p<type_identifier>,
                                           dsl::trailing_sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_list<ast::type_definition_stmts>;
  };

  static constexpr auto rule = []
  {
    return keyword::struct_ + dsl::whitespace(dsl::ascii::space)
        + dsl::p<grammar::symbol_identifier> + dsl::p<body>;
  }();

  static constexpr auto value = lexy::construct<ast::type_definition>;
};

struct outer_stmt
{
  template<typename T>
  static constexpr auto new_outer_statement =
      grammar::new_unique_ptr<T, ast::statement>;

  struct outer_stmt_error
  {
    static constexpr auto name =
        "Expected `def` to define a function, `struct` to define a newtype";
  };

  static constexpr auto rule = []
  {
    return dsl::peek(keyword::funcdef_) >> dsl::p<function_def>
        | dsl::peek(keyword::struct_) >> dsl::p<type_def>
        | dsl::error<outer_stmt_error>;
  }();
  static constexpr auto value =
      lexy::callback(new_outer_statement<ast::function_def>,
                     new_outer_statement<ast::type_definition>);
};

}  // namespace bython::grammar

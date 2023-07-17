#include <memory>
#include <string_view>
#include <vector>

#include "entrypoints.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

#include "top_level_grammar.hpp"
#include "grammar.hpp"

namespace bython::parser
{

auto module(std::string_view code, error_callback report_error)
    -> lexy::parse_result<ast::mod, error_callback>
{
  auto tree = lexy::parse<grammar::top_level<grammar::mod>>(
      lexy::string_input(code), report_error);
  return tree;
}

auto expression(std::string_view code, error_callback report_error)
    -> lexy::parse_result<std::unique_ptr<ast::expression>, error_callback>
{
  auto tree = lexy::parse<grammar::top_level<grammar::expression>>(
      lexy::string_input(code), report_error);
  return tree;
}

auto statement(std::string_view code, error_callback report_error)
    -> lexy::parse_result<std::unique_ptr<ast::statement>, error_callback>
{
  auto tree = lexy::parse<grammar::top_level<grammar::outer_stmt>>(
      lexy::string_input(code), report_error);
  return tree;
}

}  // namespace bython::parser
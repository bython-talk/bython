#pragma once

#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <lexy/action/parse.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

#include "bython/parser/top_level_grammar.hpp"

template<typename Grammar, typename T>
auto unwrap(std::string_view code) -> T
{
  INFO("Parsing '" << code << "'");

  auto expression_ast = lexy::parse<bython::grammar::top_level<Grammar>>(
      lexy::string_input(code), lexy_ext::report_error);
  if (not expression_ast.is_success()) {
    FAIL(expression_ast.errors());
  }

  auto ast = std::move(expression_ast).value();
  auto* ptr = ast.get();
  auto* upcast = dynamic_cast<T*>(ptr);

  auto as_value = std::move(*upcast);
  return as_value;
}
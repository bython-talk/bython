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
  CAPTURE(code);

  auto expression_ast = lexy::parse<bython::grammar::top_level<Grammar>>(
      lexy::string_input(code), lexy_ext::report_error);

  INFO("Errors: " << expression_ast.errors());
  REQUIRE(expression_ast.is_success());

  auto ast = std::move(expression_ast).value();
  auto* ptr = ast.get();
  auto* upcast = dynamic_cast<T*>(ptr);

  INFO("Attempting to upcast into requested type");

  CAPTURE(upcast);
  REQUIRE(upcast);

  auto as_value = std::move(*upcast);
  return as_value;
}

template<typename Grammar, typename T>
auto unwrap_failure(std::string_view code) -> void
{
  auto expression_ast = lexy::parse<bython::grammar::top_level<Grammar>>(
      lexy::string_input(code), lexy_ext::report_error);
  if (expression_ast.is_success()) {
    FAIL(code << " should not parse!");
  }
}
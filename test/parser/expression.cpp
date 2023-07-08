#include <iostream>
#include <iterator>

#include <bython/ast/expression.hpp>
#include <bython/parser/internal/expression.hpp>
#include <catch2/catch_test_macros.hpp>

#include "unwrap.hpp"

using namespace bython;

TEST_CASE("Variable")
{
  auto variable = unwrap<grammar::expression, ast::variable>("valid_variable");
  REQUIRE(variable.identifier == "valid_variable");
}

TEST_CASE("Call")
{
  SECTION("Without Arguments")
  {
    auto call = unwrap<grammar::expression, ast::call>("valid_name()");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.empty());
  }

  SECTION("With Arguments")
  {
    auto call = unwrap<grammar::expression, ast::call>("valid_name(a,b,)");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.size() == 2);
  }

  SECTION("With Nested Calls")
  {
    auto call =
        unwrap<grammar::expression, ast::call>("valid_name(inner_callee(),b,)");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.size() == 2);
  }
}
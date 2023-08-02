#include <bython/ast/compound.hpp>
#include <bython/parser/grammar.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "unwrap.hpp"

namespace ast = bython::ast;
namespace g = bython::grammar;

TEST_CASE("Function Definition")
{
  SECTION("Plain")
  {
    auto ast = unwrap_grammar<g::outer_stmt, ast::function_def>("def f(){}");
    REQUIRE(ast.name == "f");
    REQUIRE(ast.parameters.empty());
    REQUIRE(ast.body.empty());
  }

  SECTION("Parameter")
  {
    auto ast = unwrap_grammar<g::outer_stmt, ast::function_def>("def f(a,b,){}");
    REQUIRE(ast.name == "f");

    REQUIRE(ast.parameters.size() == 2);
    REQUIRE(ast.parameters.at(0).name == "a");
    REQUIRE(ast.parameters.at(1).name == "b");
    REQUIRE(ast.body.empty());
  }

  SECTION("Bad def keyword")
  {
    unwrap_grammar_failure<g::outer_stmt, ast::function_def>("f(a, b){}");
  }

  SECTION("Multi-Body")
  {
    auto ast = unwrap_grammar<g::outer_stmt, ast::function_def>(R"(
def f() {
  val x = 1 + 2;
  val y = 1 - 2;
}
)");
  }
}

TEST_CASE("Type Definition")
{
  SECTION("Simple")
  {
    auto ast = unwrap_grammar<g::outer_stmt, ast::type_definition>("struct S { }");
    REQUIRE(ast.identifier == "S");
    REQUIRE(ast.body.empty());
  }

  SECTION("Simple")
  {
    auto ast = unwrap_grammar<g::outer_stmt, ast::type_definition>("struct S { a, N }");
    REQUIRE(ast.identifier == "S");
    REQUIRE(ast.body.size() == 2);
  }
}

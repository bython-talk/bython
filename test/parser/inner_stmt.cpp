#include <bython/ast/statement.hpp>
#include <bython/matching.hpp>
#include <bython/parser/grammar.hpp>
#include <catch2/catch_test_macros.hpp>

#include "unwrap.hpp"

namespace ast = bython::ast;
namespace g = bython::grammar;
namespace m = bython::matching;
namespace grammar = bython::grammar;

TEST_CASE("Assignment")
{
  auto ast = unwrap_grammar<g::inner_stmt, ast::assignment>("val x = f;");
  auto matcher = m::lift<m::assignment>(std::string {"x"}, m::lift<m::variable>("f"));
  REQUIRE(m::matches(ast, *matcher));
}

TEST_CASE("If Statement")
{
  SECTION("Empty If")
  {
    auto ast = unwrap_grammar<g::inner_stmt, ast::conditional_branch>("if x { };");
  }

  SECTION("Single If")
  {
    auto ast =
        unwrap_grammar<g::inner_stmt, ast::conditional_branch>("if x { val y = x; val z = y; };");
  }

  SECTION("If + Elif")
  {
    auto ast = unwrap_grammar<g::inner_stmt, ast::conditional_branch>(R"(
if x {
  val y = 2;
} elif a {
  val y = 3;
};
)");
  }

  SECTION("If + Elif + Elif")
  {
    auto ast = unwrap_grammar<g::inner_stmt, ast::conditional_branch>(R"(
if x {
  val y = 2;
} elif a {
  val y = 3;
} elif b {
  val y = 4;
};
)");
  }

  SECTION("If + Else")
  {
    auto ast = unwrap_grammar<g::inner_stmt, ast::conditional_branch>(R"(
if x {
  val y = 2;
} else {
  val y = 3;
};
)");
  }

  SECTION("Missing Semicolon")
  {
    unwrap_grammar_failure<g::inner_stmt, ast::conditional_branch>(R"(
if x {
  val y = 2;
} elif z {
  val y = 3;
}
)");
  }
}
#include <bython/ast/statement.hpp>
#include <bython/parser/internal/inner_stmt.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "unwrap.hpp"

using namespace bython;

TEST_CASE("Assignment")
{
  auto ast = unwrap<grammar::inner_stmt, ast::assignment>("val x = f()");
  REQUIRE(ast.lhs == "x");
}
#include <bython/ast/statement.hpp>
#include <bython/matching.hpp>
#include <bython/parser/internal/inner_stmt.hpp>
#include <catch2/catch_test_macros.hpp>

#include "unwrap.hpp"

namespace ast = bython::ast;
namespace g = bython::grammar;
namespace m = bython::matching;

TEST_CASE("Assignment")
{
  auto ast = unwrap_grammar<g::inner_stmt, ast::assignment>("val x = f");
  REQUIRE(m::matches(
      ast, m::assignment {std::string {"x"}, m::lift<m::variable>("f")}));
}
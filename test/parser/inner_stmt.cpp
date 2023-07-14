#include <bython/ast/statement.hpp>
#include <bython/matchers.hpp>
#include <bython/parser/internal/inner_stmt.hpp>
#include <catch2/catch_test_macros.hpp>

#include "unwrap.hpp"

using namespace bython;

TEST_CASE("Assignment")
{
  auto ast = unwrap_grammar<grammar::inner_stmt, ast::assignment>("val x = f");
  REQUIRE(matching::matches(
      ast,
      matching::assignment {std::string {"x"},
                            std::make_unique<matching::variable>("f")}));
}
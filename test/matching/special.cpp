#include <iostream>
#include <iterator>

#include <bython/ast.hpp>
#include <bython/matching.hpp>
#include <catch2/catch_test_macros.hpp>

namespace ast = bython::ast;
namespace m = bython::matching;

TEST_CASE("Special Matching")
{
  SECTION("OneOf Simple")
  {
    auto code = ast::variable {"Hello World"};
    auto matcher = m::lift<m::variable>("ABC") | m::lift<m::variable>("Hello World");
    REQUIRE(m::matches(code, *matcher));
  }

  SECTION("OneOf Nested")
  {
    auto code = ast::variable {"Hello World"};
    auto matcher =
        m::lift<m::variable>("ABC") | m::lift<m::integer>(42) | m::lift<m::variable>("Hello World");
    REQUIRE(m::matches(code, *matcher));
  }
}
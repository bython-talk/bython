#include <bython/parser.hpp>
#include <catch2/catch_test_macros.hpp>

namespace parser = bython::parser;

TEST_CASE("Reach EOF")
{
  auto without_whitespace = parser::expression("variable", lexy_ext::report_error);
  REQUIRE(without_whitespace.has_value());
}

TEST_CASE("Consume Whitespace until EOF")
{
  auto with_whitespace = parser::expression("variable       ", lexy_ext::report_error);
  REQUIRE(with_whitespace.has_value());
}

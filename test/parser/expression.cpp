#include <iostream>
#include <iterator>

#include <bython/ast.hpp>
#include <bython/matchers.hpp>
#include <bython/parser/internal/expression.hpp>
#include <catch2/catch_test_macros.hpp>

#include "unwrap.hpp"

using namespace bython;

namespace m = bython::matching;

auto check_binary_op(ast::binary_operation const& binary_op,
                     ast::binary_operator op)
{
  CAPTURE(binary_op.op);
  CAPTURE(op);
  REQUIRE(binary_op.op == op);
}

auto check_comparison_op(ast::comparison const& comp_op,
                         std::vector<ast::comparison_operator> ops)

{
  CAPTURE(comp_op.ops);
  CAPTURE(ops);
  REQUIRE(comp_op.ops == ops);
}

TEST_CASE("Atoms", "[Variable]")
{
  auto variable =
      unwrap_grammar<grammar::expression, ast::variable>("valid_variable");
  REQUIRE(variable.identifier == "valid_variable");
}

TEST_CASE("Atoms", "[Call]")
{
  SECTION("Without Arguments")
  {
    auto call = unwrap_grammar<grammar::expression, ast::call>("valid_name()");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.empty());
  }

  SECTION("With Arguments")
  {
    auto call =
        unwrap_grammar<grammar::expression, ast::call>("valid_name(a,b,)");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.size() == 2);
  }

  SECTION("With Nested Calls")
  {
    auto call = unwrap_grammar<grammar::expression, ast::call>(
        "valid_name(inner_callee(),b,)");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.size() == 2);
  }
}

TEST_CASE("Compositions", "[Simple Binary Operations]")
{
  SECTION("Logical Or")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a || b");
    check_binary_op(m, ast::binary_operator::boolor);
  }

  SECTION("Logical And")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a && b");
    check_binary_op(m, ast::binary_operator::booland);
  }

  SECTION("Bit Xor")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a ^ b");
    check_binary_op(m, ast::binary_operator::bitxor_);
  }

  SECTION("Bit Or")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a | b");
    check_binary_op(m, ast::binary_operator::bitor_);
  }

  SECTION("Bit And")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a & b");
    check_binary_op(m, ast::binary_operator::bitand_);
  }

  SECTION("Minus")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a - b");
    check_binary_op(m, ast::binary_operator::minus);
  }

  SECTION("Plus")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a + b");
    check_binary_op(m, ast::binary_operator::plus);
  }

  SECTION("Modulo")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a % b");
    check_binary_op(m, ast::binary_operator::modulo);
  }

  SECTION("Division")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a / b");
    check_binary_op(m, ast::binary_operator::divide);
  }

  SECTION("Multiply")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a * b");
    check_binary_op(m, ast::binary_operator::multiply);
  }

  SECTION("Power")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a ** b");
    check_binary_op(m, ast::binary_operator::pow);
  }

  SECTION("Lesser")
  {
    auto m = unwrap_grammar<grammar::expression, ast::comparison>("a < b");
    check_comparison_op(m, {ast::comparison_operator::lsr});
  }
}

TEST_CASE("Compositions", "[Binary Operations w/ Precedence]")
{
  SECTION("Product and Sum")
  {
    auto code =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a * b + c");

    auto code_matcher = m::binary_operation(
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     ast::binary_operator::multiply,
                                     m::lift<m::variable>("b")),
        ast::binary_operator::plus,
        m::lift<m::variable>("c"));

    REQUIRE(matching::matches(code, code_matcher));
  }

  SECTION("Power of Two Check")
  {
    auto code = unwrap_grammar<grammar::expression, ast::comparison>(
        "0 == (x & (x - 1))");

    auto code_matcher = m::comparison {m::lift<m::integer>(0)}.chain(
        // =
        ast::comparison_operator::eq,
        // (x & ...)
        m::lift<m::binary_operation>(
            m::lift<m::variable>("x"),
            ast::binary_operator::bitand_,

            // (x - 1)
            m::lift<m::binary_operation>(m::lift<m::variable>("x"),
                                         ast::binary_operator::minus,
                                         m::lift<m::integer>(1))));

    REQUIRE(matching::matches(code, code_matcher));
  }
}
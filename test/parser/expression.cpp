#include <iostream>
#include <iterator>

#include <bython/ast.hpp>
#include <bython/matchers.hpp>
#include <bython/parser/internal/expression.hpp>
#include <catch2/catch_test_macros.hpp>

#include "unwrap.hpp"

using namespace bython;

auto check_binary_op(ast::binary_operation const& binary_op,
                     ast::binary_operation::binop op)
{
  CAPTURE(binary_op.op);
  CAPTURE(op);
  REQUIRE(binary_op.op == op);
}

auto check_comparison_op(ast::comparison const& comp_op,
                         std::vector<ast::comparison::compop> ops)

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
    check_binary_op(m, ast::binary_operation::binop::boolor);
  }

  SECTION("Logical And")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a && b");
    check_binary_op(m, ast::binary_operation::binop::booland);
  }

  SECTION("Bit Xor")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a ^ b");
    check_binary_op(m, ast::binary_operation::binop::bitxor_);
  }

  SECTION("Bit Or")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a | b");
    check_binary_op(m, ast::binary_operation::binop::bitor_);
  }

  SECTION("Bit And")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a & b");
    check_binary_op(m, ast::binary_operation::binop::bitand_);
  }

  SECTION("Minus")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a - b");
    check_binary_op(m, ast::binary_operation::binop::minus);
  }

  SECTION("Plus")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a + b");
    check_binary_op(m, ast::binary_operation::binop::plus);
  }

  SECTION("Modulo")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a % b");
    check_binary_op(m, ast::binary_operation::binop::modulo);
  }

  SECTION("Division")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a / b");
    check_binary_op(m, ast::binary_operation::binop::divide);
  }

  SECTION("Multiply")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a * b");
    check_binary_op(m, ast::binary_operation::binop::multiply);
  }

  SECTION("Power")
  {
    auto m =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a ** b");
    check_binary_op(m, ast::binary_operation::binop::pow);
  }

  SECTION("Lesser")
  {
    auto m = unwrap_grammar<grammar::expression, ast::comparison>("a < b");
    check_comparison_op(m, {ast::comparison::compop::lsr});
  }
}

TEST_CASE("Compositions", "[Binary Operations w/ Precedence]")
{
  SECTION("Product and Sum")
  {
    auto code =
        unwrap_grammar<grammar::expression, ast::binary_operation>("a * b + c");

    auto code_matcher = matching::binary_operation(
        std::make_unique<matching::binary_operation>(
            std::make_unique<matching::variable>("a"),
            ast::binary_operation::binop::multiply,
            std::make_unique<matching::variable>("b")),
        ast::binary_operation::binop::plus,
        std::make_unique<matching::variable>("c"));

    REQUIRE(matching::matches(code, code_matcher));
  }

  SECTION("Power of Two Check")
  {
    auto m = unwrap_grammar<grammar::expression, ast::comparison>(
        "0 == (x & (x - 1))");
    REQUIRE(matching::matches(
        m,
        matching::comparison {std::make_unique<matching::integer>(0)}.chain(
            // =
            ast::comparison::compop::eq,
            // (x & ...)
            std::make_unique<matching::binary_operation>(
                std::make_unique<matching::variable>("x"),
                ast::binary_operation::binop::bitand_,

                // (x - 1)
                std::make_unique<matching::binary_operation>(
                    std::make_unique<matching::variable>("x"),
                    ast::binary_operation::binop::minus,
                    std::make_unique<matching::integer>(1))))));
  }
}
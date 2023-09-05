#include <iostream>
#include <iterator>

#include <catch2/catch_test_macros.hpp>

#include "bython/ast.hpp"
#include "bython/lexy_frontend/grammar.hpp"
#include "bython/matching.hpp"
#include "unwrap.hpp"

namespace ast = bython::ast;
namespace g = bython::grammar;
namespace m = bython::matching;

TEST_CASE("Atoms", "[Variable]")
{
  auto variable = unwrap_grammar<g::expression, ast::variable>("valid_variable");
  auto matcher = m::lift<m::variable>("valid_variable");
  REQUIRE(m::matches(variable, *matcher));
}

TEST_CASE("Atoms", "[Call]")
{
  SECTION("Without Arguments")
  {
    auto call = unwrap_grammar<g::expression, ast::call>("valid_name()");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.empty());
  }

  SECTION("With Arguments")
  {
    auto call = unwrap_grammar<g::expression, ast::call>("valid_name(a,b,)");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.size() == 2);
  }

  SECTION("With Nested Calls")
  {
    auto call = unwrap_grammar<g::expression, ast::call>("valid_name(inner_callee(),b,)");

    REQUIRE(call.callee == "valid_name");
    REQUIRE(call.arguments.size() == 2);
  }
}

TEST_CASE("Compositions", "[Simple Binary Operations]")
{
  SECTION("Logical Or")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a || b");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                                m::lift<m::binary_operator>(ast::binop_tag::boolor),
                                                m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Logical And")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a && b");
    auto matcher =
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     m::lift<m::binary_operator>(ast::binop_tag::booland),
                                     m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Bit Xor")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a ^ b");
    auto matcher =
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     m::lift<m::binary_operator>(ast::binop_tag::bitxor_),
                                     m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Bit Or")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a | b");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                                m::lift<m::binary_operator>(ast::binop_tag::bitor_),
                                                m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Bit And")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a & b");
    auto matcher =
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     m::lift<m::binary_operator>(ast::binop_tag::bitand_),
                                     m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Bitshift Right")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a >> b");
    auto matcher =
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     m::lift<m::binary_operator>(ast::binop_tag::bitshift_right_),
                                     m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Bitshift Left")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a << b");
    auto matcher =
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     m::lift<m::binary_operator>(ast::binop_tag::bitshift_left_),
                                     m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Minus")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a - b");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                                m::lift<m::binary_operator>(ast::binop_tag::minus),
                                                m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Plus")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a + b");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                                m::lift<m::binary_operator>(ast::binop_tag::plus),
                                                m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Modulo")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a % b");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                                m::lift<m::binary_operator>(ast::binop_tag::modulo),
                                                m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Division")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a / b");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                                m::lift<m::binary_operator>(ast::binop_tag::divide),
                                                m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Multiply")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a * b");
    auto matcher =
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     m::lift<m::binary_operator>(ast::binop_tag::multiply),
                                     m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Power")
  {
    auto m = unwrap_grammar<g::expression, ast::binary_operation>("a ** b");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                                m::lift<m::binary_operator>(ast::binop_tag::pow),
                                                m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, *matcher));
  }

  SECTION("Lesser")
  {
    auto m = unwrap_grammar<g::expression, ast::comparison>("a < b");
    auto matcher = m::comparison(m::lift<m::variable>("a"))
                       .chain(m::lift<m::comparison_operator>(ast::comparison_operator_tag::lsr),
                              m::lift<m::variable>("b"));
    REQUIRE(m::matches(m, matcher));
  }

  SECTION("As")
  {
    auto code = unwrap_grammar<g::expression, ast::binary_operation>("x as u32");
    auto matcher = m::lift<m::binary_operation>(m::lift<m::variable>("x"),
                                                m::lift<m::binary_operator>(ast::binop_tag::as),
                                                m::lift<m::variable>("u32"));

    REQUIRE(m::matches(code, *matcher));
  }
}

TEST_CASE("Compositions", "[Binary Operations w/ Precedence]")
{
  SECTION("Product and Sum")
  {
    auto code = unwrap_grammar<g::expression, ast::binary_operation>("a * b + c");

    auto code_matcher = m::lift<m::binary_operation>(
        m::lift<m::binary_operation>(m::lift<m::variable>("a"),
                                     m::lift<m::binary_operator>(ast::binop_tag::multiply),
                                     m::lift<m::variable>("b")),
        m::lift<m::binary_operator>(ast::binop_tag::plus),
        m::lift<m::variable>("c"));

    REQUIRE(m::matches(code, *code_matcher));
  }

  SECTION("Power of Two Check")
  {
    auto code = unwrap_grammar<g::expression, ast::comparison>("0 == (x & (x - 1))");

    auto code_matcher = m::comparison {m::lift<m::integer>(0)}.chain(
        // ==
        m::lift<m::comparison_operator>(ast::comparison_operator_tag::eq),
        // (x & ...)
        m::lift<m::binary_operation>(
            m::lift<m::variable>("x"),
            m::lift<m::binary_operator>(ast::binop_tag::bitand_),

            // (x - 1)
            m::lift<m::binary_operation>(m::lift<m::variable>("x"),
                                         m::lift<m::binary_operator>(ast::binop_tag::minus),
                                         m::lift<m::integer>(1))));

    REQUIRE(m::matches(code, code_matcher));
  }

  SECTION("All the Unary Operators")
  {
    auto code = unwrap_grammar<g::expression, ast::binary_operation>("~x + +x - -x");
    auto code_matcher = m::lift<m::binary_operation>(
        m::lift<m::binary_operation>(
            m::lift<m::unary_operation>(m::lift<m::unary_operator>(ast::unop_tag::bitnegate),
                                        m::lift<m::variable>("x")),
            m::lift<m::binary_operator>(ast::binop_tag::plus),
            m::lift<m::unary_operation>(m::lift<m::unary_operator>(ast::unop_tag::plus),
                                        m::lift<m::variable>("x"))),
        m::lift<m::binary_operator>(ast::binop_tag::minus),
        m::lift<m::unary_operation>(m::lift<m::unary_operator>(ast::unop_tag::minus),
                                    m::lift<m::variable>("x")));

    REQUIRE(m::matches(code, *code_matcher));
  }
}

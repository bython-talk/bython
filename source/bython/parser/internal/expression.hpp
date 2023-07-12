#pragma once

#include <memory>
#include <string>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include "bython/ast/expression.hpp"
#include "primitives.hpp"

namespace bython::grammar
{

template<typename T>
static constexpr auto new_expression =
    grammar::new_unique_ptr<T, ast::expression>;

struct nested_expression : lexy::transparent_production
{
  static constexpr auto rule = dsl::recurse<struct expr_prod>;
  static constexpr auto value = lexy::forward<std::unique_ptr<ast::expression>>;
};

struct var_or_call
{
  struct parameter_list
  {
    static constexpr auto rule = []
    {
      return dsl::round_bracketed.opt_list(dsl::p<nested_expression>,
                                           dsl::trailing_sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_list<ast::expressions>;
  };

  static constexpr auto rule = []
  { return dsl::p<symbol_identifier> >> dsl::if_(dsl::p<parameter_list>); }();

  static constexpr auto value = lexy::callback(
      lexy::construct<ast::variable> | new_expression<ast::variable>,
      lexy::construct<ast::call> | new_expression<ast::call>);
};

struct parenthesized
{
  static constexpr auto rule = []
  { return dsl::parenthesized(dsl::p<nested_expression>); }();

  static constexpr auto value = lexy::forward<std::unique_ptr<ast::expression>>;
};

namespace operators
{
static constexpr auto pow =
    dsl::op<ast::binary_operation::binop::pow>(LEXY_LIT("**"));

static constexpr auto unary_plus =
    dsl::op<ast::unary_operation::unop::plus>(dsl::lit_c<'+'>);
static constexpr auto unary_minus =
    dsl::op<ast::unary_operation::unop::minus>(dsl::lit_c<'-'>);
static constexpr auto unary_bitnegate =
    dsl::op<ast::unary_operation::unop::bitnegate>(dsl::lit_c<'~'>);

static constexpr auto mul = dsl::op<ast::binary_operation::binop::multiply>(
    dsl::not_followed_by(LEXY_LIT("*"), dsl::lit_c<'*'>));
static constexpr auto div =
    dsl::op<ast::binary_operation::binop::divide>(dsl::lit_c<'/'>);
static constexpr auto modulo =
    dsl::op<ast::binary_operation::binop::modulo>(dsl::lit_c<'%'>);

static constexpr auto add =
    dsl::op<ast::binary_operation::binop::plus>(dsl::lit_c<'+'>);
static constexpr auto minus =
    dsl::op<ast::binary_operation::binop::minus>(dsl::lit_c<'-'>);

static constexpr auto bitand_ = dsl::op<ast::binary_operation::binop::bitand_>(
    dsl::not_followed_by(LEXY_LIT("&"), dsl::lit_c<'&'>));
static constexpr auto bitor_ = dsl::op<ast::binary_operation::binop::bitor_>(
    dsl::not_followed_by(LEXY_LIT("|"), dsl::lit_c<'|'>));
static constexpr auto bitxor_ =
    dsl::op<ast::binary_operation::binop::bitxor_>(dsl::lit_c<'^'>);

static constexpr auto logical_and =
    dsl::op<ast::binary_operation::binop::booland>(LEXY_LIT("&&"));
static constexpr auto logical_or =
    dsl::op<ast::binary_operation::binop::boolor>(LEXY_LIT("||"));

}  // namespace operators

struct expr_prod : lexy::expression_production
{
  struct expression_error
  {
    static constexpr auto name = "unknown expression";
  };

  static constexpr auto atom = []
  {
    return dsl::p<var_or_call> | dsl::p<parenthesized>
        | dsl::error<expression_error>;
  }();

  struct math_power : dsl::infix_op_right
  {
    static constexpr auto op = operators::pow;
    using operand = dsl::atom;
  };

  struct unary_operation : dsl::prefix_op
  {
    static constexpr auto op = operators::unary_plus / operators::unary_minus
        / operators::unary_bitnegate;
    using operand = math_power;
  };

  struct math_product : dsl::infix_op_right
  {
    static constexpr auto op =
        operators::mul / operators::div / operators::modulo;
    using operand = unary_operation;
  };

  struct math_sum : dsl::infix_op_left
  {
    static constexpr auto op = operators::add / operators::minus;
    using operand = math_product;
  };

  struct bit_and : dsl::infix_op_left
  {
    static constexpr auto op = operators::bitand_;
    using operand = math_sum;
  };

  struct bit_or : dsl::infix_op_left
  {
    static constexpr auto op = operators::bitor_;
    using operand = bit_and;
  };

  struct bit_xor : dsl::infix_op_left
  {
    static constexpr auto op = operators::bitxor_;
    using operand = bit_or;
  };

  struct logical_and : dsl::infix_op_left
  {
    static constexpr auto op = operators::logical_and;
    using operand = bit_xor;
  };

  struct logical_or : dsl::infix_op_left
  {
    static constexpr auto op = operators::logical_or;
    using operand = logical_and;
  };

  using operation = logical_or;

  static constexpr auto value =
      lexy::callback(new_expression<ast::call>,
                     new_expression<ast::variable>,
                     new_expression<ast::binary_operation>,
                     new_expression<ast::unary_operation>,
                     lexy::forward<std::unique_ptr<ast::expression>>);
};

struct expression
{
  static constexpr auto rule = dsl::p<expr_prod>;
  static constexpr auto value = lexy::forward<std::unique_ptr<ast::expression>>;
};
}  // namespace bython::grammar

#pragma once

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include <bython/ast.hpp>

namespace bython::grammar
{

namespace dsl = lexy::dsl;

namespace internal
{
static constexpr auto identifier = dsl::identifier(
    dsl::ascii::alpha_underscore, dsl::ascii::alpha_digit_underscore);

}  // namespace internal

namespace keyword
{
static constexpr auto funcdef_ = LEXY_KEYWORD("def", internal::identifier);
static constexpr auto return_ = LEXY_KEYWORD("return", internal::identifier);
static constexpr auto variable_ = LEXY_KEYWORD("val", internal::identifier);
static constexpr auto struct_ = LEXY_KEYWORD("struct", internal::identifier);
}  // namespace keyword

namespace internal
{
static constexpr auto reserved_identifier = identifier.reserve(
    keyword::funcdef_, keyword::return_, keyword::variable_, keyword::struct_);
}

struct symbol_identifier
{
  static constexpr auto rule = internal::reserved_identifier;
  static constexpr auto value = lexy::as_string<std::string>;
};

struct type_identifier
{
  static constexpr auto rule = internal::reserved_identifier;
  static constexpr auto value = lexy::as_string<std::string>;
};

template<typename T, typename U>
static constexpr auto new_unique_ptr = lexy::new_<T, std::unique_ptr<U>>;

/* === Expressions === */

template<typename T>
static constexpr auto new_expression =
    grammar::new_unique_ptr<T, ast::expression>;

struct nested_expression : lexy::transparent_production
{
  static constexpr auto rule = dsl::recurse<struct expr_prod>;
  static constexpr auto value = lexy::forward<std::unique_ptr<ast::expression>>;
};

struct integer : lexy::token_production
{
  static constexpr auto rule =
      LEXY_LIT("0x") >> dsl::integer<int, dsl::hex> | dsl::integer<int>;

  static constexpr auto value =
      lexy::construct<ast::integer> | new_expression<ast::integer>;
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
static constexpr auto pow = dsl::op<ast::binop_tag::pow>(LEXY_LIT("**"));

static constexpr auto unary_plus =
    dsl::op<ast::unop_tag::plus>(dsl::lit_c<'+'>);
static constexpr auto unary_minus =
    dsl::op<ast::unop_tag::minus>(dsl::lit_c<'-'>);
static constexpr auto unary_bitnegate =
    dsl::op<ast::unop_tag::bitnegate>(dsl::lit_c<'~'>);

static constexpr auto mul = dsl::op<ast::binop_tag::multiply>(
    dsl::not_followed_by(LEXY_LIT("*"), dsl::lit_c<'*'>));
static constexpr auto div = dsl::op<ast::binop_tag::divide>(dsl::lit_c<'/'>);
static constexpr auto modulo = dsl::op<ast::binop_tag::modulo>(dsl::lit_c<'%'>);

static constexpr auto add = dsl::op<ast::binop_tag::plus>(dsl::lit_c<'+'>);
static constexpr auto minus = dsl::op<ast::binop_tag::minus>(dsl::lit_c<'-'>);

static constexpr auto bitshift_right =
    dsl::op<ast::binop_tag::bitshift_right_>(LEXY_LIT(">>"));
static constexpr auto bitshift_left =
    dsl::op<ast::binop_tag::bitshift_left_>(LEXY_LIT("<<"));

static constexpr auto bitand_ = dsl::op<ast::binop_tag::bitand_>(
    dsl::not_followed_by(LEXY_LIT("&"), dsl::lit_c<'&'>));
static constexpr auto bitor_ = dsl::op<ast::binop_tag::bitor_>(
    dsl::not_followed_by(LEXY_LIT("|"), dsl::lit_c<'|'>));
static constexpr auto bitxor_ =
    dsl::op<ast::binop_tag::bitxor_>(dsl::lit_c<'^'>);

static constexpr auto logical_and =
    dsl::op<ast::binop_tag::booland>(LEXY_LIT("&&"));
static constexpr auto logical_or =
    dsl::op<ast::binop_tag::boolor>(LEXY_LIT("||"));

static constexpr auto lsr = dsl::op<ast::comparison_operator_tag::lsr>(
    dsl::not_followed_by(dsl::lit_c<'<'>, dsl::lit_c<'='>));

static constexpr auto leq =
    dsl::op<ast::comparison_operator_tag::leq>(LEXY_LIT("<="));

static constexpr auto geq =
    dsl::op<ast::comparison_operator_tag::geq>(LEXY_LIT(">="));

static constexpr auto grt = dsl::op<ast::comparison_operator_tag::grt>(
    dsl::not_followed_by(dsl::lit_c<'>'>, dsl::lit_c<'='>));

static constexpr auto eq =
    dsl::op<ast::comparison_operator_tag::eq>(LEXY_LIT("=="));

static constexpr auto neq =
    dsl::op<ast::comparison_operator_tag::neq>(LEXY_LIT("!="));

}  // namespace operators

struct expr_prod : lexy::expression_production
{
  struct expression_error
  {
    static constexpr auto name = "unknown expression";
  };

  static constexpr auto atom = []
  {
    return dsl::p<var_or_call> | dsl::p<integer> | dsl::p<parenthesized>
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

  struct bitshift : dsl::infix_op_left
  {
    static constexpr auto op =
        operators::bitshift_right / operators::bitshift_left;
    using operand = math_sum;
  };

  struct bit_and : dsl::infix_op_left
  {
    static constexpr auto op = operators::bitand_;
    using operand = bitshift;
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

  struct comparison : dsl::infix_op_list
  {
    static constexpr auto op = operators::neq / operators::eq / operators::grt
        / operators::geq / operators::leq / operators::lsr;
    using operand = logical_or;
  };

  using operation = comparison;

  static constexpr auto value =
      lexy::fold_inplace<std::unique_ptr<ast::comparison>>(
          []
          {
            auto empty_comp =
                ast::comparison {ast::expressions {},
                                 std::vector<ast::comparison_operator_tag> {}};
            return std::make_unique<ast::comparison>(std::move(empty_comp));
          },
          [](std::unique_ptr<ast::comparison>& comparison,
             std::unique_ptr<ast::expression> expr)
          { comparison->add_operand(std::move(expr)); },
          [](std::unique_ptr<ast::comparison>& comparison,
             ast::comparison_operator_tag cmp)
          { comparison->add_operator(std::move(cmp)); })
      >> lexy::callback(new_expression<ast::call>,
                        new_expression<ast::variable>,
                        new_expression<ast::integer>,
                        new_expression<ast::binary_operation>,
                        new_expression<ast::unary_operation>,
                        new_expression<ast::comparison>,
                        lexy::forward<std::unique_ptr<ast::expression>>);
};

struct expression
{
  static constexpr auto rule = dsl::p<expr_prod>;
  static constexpr auto value = lexy::forward<std::unique_ptr<ast::expression>>;
};

/* === Inner Statements === */

struct assignment
{
  static constexpr auto rule = []
  {
    return keyword::variable_ + dsl::whitespace(dsl::ascii::space)
        + dsl::p<grammar::symbol_identifier>
        + LEXY_LIT("=") + dsl::p<expression>;
  }();

  static constexpr auto value = lexy::construct<ast::assignment>
      | lexy::new_<ast::assignment, std::unique_ptr<ast::statement>>;
};

struct inner_stmt
{
  static constexpr auto rule = []
  { return dsl::peek(keyword::variable_) >> dsl::p<assignment>; }();

  static constexpr auto value = lexy::forward<std::unique_ptr<ast::statement>>;
};

/* === Outer Statements === */

struct compound_body
{
  static constexpr auto rule = []
  {
    auto terminator = dsl::terminator(dsl::semicolon).limit(dsl::lit_c<'}'>);
    return dsl::curly_bracketed.opt_list(terminator(dsl::p<inner_stmt>));
  }();

  static constexpr auto value = lexy::as_list<ast::statements>;
};

struct function_def
{
  struct parameter
  {
    static constexpr auto rule = [] { return dsl::p<symbol_identifier>; }();
    static constexpr auto value = lexy::construct<ast::parameter>;
  };

  struct parameters
  {
    static constexpr auto rule = []
    {
      return dsl::round_bracketed.opt_list(dsl::p<parameter>,
                                           dsl::trailing_sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_list<ast::parameters>;
  };

  static constexpr auto rule = []
  {
    return keyword::funcdef_ + dsl::whitespace(dsl::ascii::space)
        + dsl::p<grammar::symbol_identifier> + dsl::p<parameters>
        + dsl::p<compound_body>;
  }();

  static constexpr auto value = lexy::construct<ast::function_def>;
};

struct type_def
{
  struct body
  {
    static constexpr auto rule = []
    {
      return dsl::curly_bracketed.opt_list(dsl::p<type_identifier>,
                                           dsl::trailing_sep(dsl::comma));
    }();

    static constexpr auto value = lexy::as_list<ast::type_definition_stmts>;
  };

  static constexpr auto rule = []
  {
    return keyword::struct_ + dsl::whitespace(dsl::ascii::space)
        + dsl::p<grammar::symbol_identifier> + dsl::p<body>;
  }();

  static constexpr auto value = lexy::construct<ast::type_definition>;
};

struct outer_stmt
{
  template<typename T>
  static constexpr auto new_outer_statement =
      grammar::new_unique_ptr<T, ast::statement>;

  struct outer_stmt_error
  {
    static constexpr auto name =
        "Expected `def` to define a function, `struct` to define a newtype";
  };

  static constexpr auto rule = []
  {
    return dsl::peek(keyword::funcdef_) >> dsl::p<function_def>
        | dsl::peek(keyword::struct_) >> dsl::p<type_def>
        | dsl::error<outer_stmt_error>;
  }();
  static constexpr auto value =
      lexy::callback(new_outer_statement<ast::function_def>,
                     new_outer_statement<ast::type_definition>);
};

struct mod
{
  static constexpr auto rule = []
  {
    // top-level statements are separated by newline
    return dsl::terminator(dsl::newline).list(dsl::p<outer_stmt>);
  }();

  static constexpr auto value =
      lexy::as_list<ast::statements> >> lexy::construct<ast::mod>;
};
}  // namespace bython::grammar
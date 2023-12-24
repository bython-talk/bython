#include <concepts>
#include <iostream>
#include <iterator>
#include <optional>
#include <ostream>
#include <type_traits>

#include "lexy.hpp"

#include <boost/container_hash/hash.hpp>  // uuid hasher
#include <boost/uuid/uuid.hpp>  // uuid class
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/callback/adapter.hpp>
#include <lexy/callback/object.hpp>
#include <lexy/dsl.hpp>
#include <lexy/dsl/expression.hpp>
#include <lexy/dsl/literal.hpp>
#include <lexy/dsl/option.hpp>
#include <lexy/error.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/input_location.hpp>
#include <lexy_ext/report_error.hpp>

#include "bython/ast.hpp"
#include "bython/ast/expression.hpp"
#include "bython/ast/statement.hpp"

namespace dsl = lexy::dsl;

namespace ast = bython::ast;
namespace p = bython::parser;

template<typename T>
struct top_level
{
  static constexpr auto whitespace = dsl::ascii::space | LEXY_LIT("#") >> dsl::until(dsl::newline)
      | dsl::backslash >> dsl::newline;

  static constexpr auto rule = T::rule + dsl::whitespace(whitespace) + dsl::eof;
  static constexpr auto value = T::value;
};

template<typename T>
struct is_unique_ptr : std::false_type
{
};

template<typename T, typename D>
struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type
{
};

template<typename Input>
struct lexy_grammar
{
  template<typename Production, typename Value>
  struct with_span : lexy::transparent_production
  {
    static constexpr auto rule = dsl::position(dsl::p<Production>) >> dsl::position;

    static constexpr auto value = lexy::callback_with_state<Value>(
        [](auto& state, auto& startptr, Value node, auto& endptr)
        {
          auto begin = lexy::get_input_location(state.input, startptr);
          auto end = lexy::get_input_location(state.input, endptr);

          using LexySpan = typename decltype(state.span_lookup)::mapped_type;
          auto node_span = LexySpan {.begin = begin, .end = end};

          if constexpr (is_unique_ptr<std::remove_cvref_t<Value>>::value) {
            state.span_lookup.insert({node->uuid, std::move(node_span)});
          } else {
            state.span_lookup.insert({node.uuid, std::move(node_span)});
          }
          return node;
        });
  };

  static constexpr auto identifier =
      dsl::identifier(dsl::ascii::alpha_underscore, dsl::ascii::alpha_digit_underscore);

  struct keyword
  {
    static constexpr auto funcdef_ = LEXY_KEYWORD("def", identifier);
    static constexpr auto return_ = LEXY_KEYWORD("return", identifier);
    static constexpr auto variable_ = LEXY_KEYWORD("val", identifier);
    static constexpr auto struct_ = LEXY_KEYWORD("struct", identifier);

    static constexpr auto if_ = LEXY_KEYWORD("if", identifier);
    static constexpr auto elif_ = LEXY_KEYWORD("elif", identifier);
    static constexpr auto else_ = LEXY_KEYWORD("else", identifier);

    static constexpr auto as_ = LEXY_KEYWORD("as", identifier);
    static constexpr auto discard_ = LEXY_KEYWORD("discard", identifier);

    static constexpr auto reserved =
        identifier.reserve(funcdef_, return_, variable_, struct_, if_, elif_, else_, as_, discard_);
  };

  struct symbol_identifier
  {
    static constexpr auto rule = keyword::reserved;
    static constexpr auto value = lexy::as_string<std::string>;
  };

  struct type_identifier
  {
    static constexpr auto rule = keyword::reserved;
    static constexpr auto value = lexy::as_string<std::string>;
  };

  template<typename T, typename U>
  static constexpr auto new_unique_ptr = lexy::new_<T, std::unique_ptr<U>>;

  /* === Expressions === */
  template<typename T>
  static constexpr auto new_expression = new_unique_ptr<T, ast::expression>;

  struct expr_prod;
  struct nested_expression : lexy::transparent_production
  {
    static constexpr auto rule = dsl::recurse<expr_prod>;
    static constexpr auto value = lexy::forward<ast::expression_ptr>;
  };

  struct integer : lexy::token_production
  {
    template<std::integral T>
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule = []
      {
        auto constexpr suffix =  // Hexadecimal: 0x0F
            LEXY_LIT("0x") >> dsl::integer<T, dsl::hex> |
            // Explicit Decimal: 0d09
            LEXY_LIT("0d") >> dsl::integer<T, dsl::decimal> |
            // Octal: 0o07
            LEXY_LIT("0o") >> dsl::integer<T, dsl::octal> |
            // Binary: 0b01
            LEXY_LIT("0b") >> dsl::integer<T, dsl::binary> |
            // Decimal
            dsl::integer<T, dsl::decimal>;

        if constexpr (std::is_unsigned_v<T>) {
          auto constexpr preamble = dsl::peek(dsl::lit_c<'+'> / dsl::digit<dsl::decimal>);
          return preamble >> (dsl::plus_sign + suffix);
        } else {
          auto constexpr preamble = dsl::peek(dsl::lit_c<'-'>);
          return preamble >> (dsl::minus_sign + suffix);
        }
      }();

      static constexpr auto value = lexy::as_integer<T>;
    };

    struct signed_int : lexy::transparent_production
    {
      static constexpr auto rule = dsl::p<inner<int64_t>>;
      static constexpr auto value =
          lexy::construct<ast::signed_integer> | new_expression<ast::signed_integer>;
    };

    struct unsigned_int : lexy::transparent_production
    {
      static constexpr auto rule = dsl::p<inner<uint64_t>>;
      static constexpr auto value =
          lexy::construct<ast::unsigned_integer> | new_expression<ast::unsigned_integer>;
    };

    static constexpr auto rule = dsl::p<with_span<signed_int, ast::expression_ptr>>
        | dsl::p<with_span<unsigned_int, ast::expression_ptr>>;
    static constexpr auto value = lexy::forward<ast::expression_ptr>;
  };

  struct argument_list
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule =
          dsl::round_bracketed.opt_list(dsl::p<nested_expression>, dsl::trailing_sep(dsl::comma));
      static constexpr auto value = lexy::as_list<ast::expressions> >> lexy::bind(
                                        lexy::construct<ast::argument_list>, lexy::_1.or_default());
    };

    static constexpr auto rule = dsl::p<with_span<inner, ast::argument_list>>;
    static constexpr auto value = lexy::forward<ast::argument_list>;
  };

  struct var_or_call
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule = dsl::p<symbol_identifier> >> dsl::if_(dsl::p<argument_list>);
      static constexpr auto value =
          lexy::callback(lexy::construct<ast::variable> | new_expression<ast::variable>,
                         lexy::construct<ast::call> | new_expression<ast::call>);
    };

    static constexpr auto rule = dsl::p<with_span<inner, ast::expression_ptr>>;
    static constexpr auto value = lexy::forward<ast::expression_ptr>;
  };

  struct parenthesized
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule = dsl::parenthesized(dsl::p<nested_expression>);
      static constexpr auto value = lexy::forward<ast::expression_ptr>;
    };

    static constexpr auto rule = dsl::p<with_span<inner, ast::expression_ptr>>;
    static constexpr auto value = lexy::forward<ast::expression_ptr>;
  };

  struct binary_operators
  {
    static constexpr auto pow = dsl::op<ast::binop_tag::pow>(LEXY_LIT("**"));

    static constexpr auto unary_plus = dsl::op<ast::unop_tag::plus>(dsl::lit_c<'+'>);
    static constexpr auto unary_minus = dsl::op<ast::unop_tag::minus>(dsl::lit_c<'-'>);
    static constexpr auto unary_bitnegate = dsl::op<ast::unop_tag::bitnegate>(dsl::lit_c<'~'>);

    static constexpr auto mul =
        dsl::op<ast::binop_tag::multiply>(dsl::not_followed_by(LEXY_LIT("*"), dsl::lit_c<'*'>));
    static constexpr auto div = dsl::op<ast::binop_tag::divide>(dsl::lit_c<'/'>);
    static constexpr auto modulo = dsl::op<ast::binop_tag::modulo>(dsl::lit_c<'%'>);

    static constexpr auto add = dsl::op<ast::binop_tag::plus>(dsl::lit_c<'+'>);
    static constexpr auto minus = dsl::op<ast::binop_tag::minus>(dsl::lit_c<'-'>);

    static constexpr auto bitshift_right = dsl::op<ast::binop_tag::bitshift_right_>(LEXY_LIT(">>"));
    static constexpr auto bitshift_left = dsl::op<ast::binop_tag::bitshift_left_>(LEXY_LIT("<<"));

    static constexpr auto bitand_ =
        dsl::op<ast::binop_tag::bitand_>(dsl::not_followed_by(LEXY_LIT("&"), dsl::lit_c<'&'>));
    static constexpr auto bitor_ =
        dsl::op<ast::binop_tag::bitor_>(dsl::not_followed_by(LEXY_LIT("|"), dsl::lit_c<'|'>));
    static constexpr auto bitxor_ = dsl::op<ast::binop_tag::bitxor_>(dsl::lit_c<'^'>);

    static constexpr auto logical_and = dsl::op<ast::binop_tag::booland>(LEXY_LIT("&&"));
    static constexpr auto logical_or = dsl::op<ast::binop_tag::boolor>(LEXY_LIT("||"));

    static constexpr auto lsr = dsl::op<ast::comparison_operator_tag::lsr>(
        dsl::not_followed_by(dsl::lit_c<'<'>, dsl::lit_c<'='>));

    static constexpr auto leq = dsl::op<ast::comparison_operator_tag::leq>(LEXY_LIT("<="));

    static constexpr auto geq = dsl::op<ast::comparison_operator_tag::geq>(LEXY_LIT(">="));

    static constexpr auto grt = dsl::op<ast::comparison_operator_tag::grt>(
        dsl::not_followed_by(dsl::lit_c<'>'>, dsl::lit_c<'='>));

    static constexpr auto eq = dsl::op<ast::comparison_operator_tag::eq>(LEXY_LIT("=="));

    static constexpr auto neq = dsl::op<ast::comparison_operator_tag::neq>(LEXY_LIT("!="));
  };

  struct expr_prod : lexy::expression_production
  {
    struct expression_error
    {
      static constexpr auto name = "unknown expression";
    };

    static constexpr auto atom = dsl::p<var_or_call> | dsl::p<parenthesized> | dsl::p<integer>
        | dsl::error<expression_error>;

    struct math_power : dsl::infix_op_right
    {
      static constexpr auto op = binary_operators::pow;
      using operand = dsl::atom;
    };

    struct unary_operation : dsl::prefix_op
    {
      static constexpr auto op = binary_operators::unary_plus / binary_operators::unary_minus
          / binary_operators::unary_bitnegate;
      using operand = math_power;
    };

    struct math_product : dsl::infix_op_right
    {
      static constexpr auto op =
          binary_operators::mul / binary_operators::div / binary_operators::modulo;
      using operand = unary_operation;
    };

    struct math_sum : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::add / binary_operators::minus;
      using operand = math_product;
    };

    struct bitshift : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::bitshift_right / binary_operators::bitshift_left;
      using operand = math_sum;
    };

    struct bit_and : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::bitand_;
      using operand = bitshift;
    };

    struct bit_or : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::bitor_;
      using operand = bit_and;
    };

    struct bit_xor : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::bitxor_;
      using operand = bit_or;
    };

    struct logical_and : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::logical_and;
      using operand = bit_xor;
    };

    struct logical_or : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::logical_or;
      using operand = logical_and;
    };

    struct comparison : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::neq / binary_operators::eq
          / binary_operators::grt / binary_operators::geq / binary_operators::leq
          / binary_operators::lsr;
      using operand = logical_or;
    };

    using operation = comparison;

    static constexpr auto value = lexy::callback(new_expression<ast::call>,
                                                 new_expression<ast::variable>,
                                                 new_expression<ast::signed_integer>,
                                                 new_expression<ast::binary_operation>,
                                                 new_expression<ast::unary_operation>,
                                                 new_expression<ast::comparison>,
                                                 lexy::forward<ast::expression_ptr>);
  };

  struct expression
  {
    static constexpr auto rule = dsl::p<expr_prod>;
    static constexpr auto value = lexy::forward<ast::expression_ptr>;
  };

  /* === Statements === */
  template<typename T>
  static constexpr auto new_statement = new_unique_ptr<T, ast::statement>;

  /* === Inner Statements === */
  struct inner_compound_body;

  struct branch_body
  {
    static constexpr auto rule = dsl::recurse<struct inner_compound_body>;
    static constexpr auto value = lexy::forward<ast::statements>;
  };

  struct dependent_branch
  {
    static constexpr auto rule = []
    {
      auto another_elif = dsl::p<nested_expression> + dsl::p<branch_body>
          + dsl::opt(dsl::recurse_branch<struct dependent_branch>);
      auto terminating_else = dsl::p<branch_body>;

      return keyword::elif_ >> another_elif | keyword::else_ >> terminating_else;
    }();

    static constexpr auto value = lexy::callback(
        lexy::construct<ast::conditional_branch> | new_statement<ast::conditional_branch>,
        lexy::construct<ast::unconditional_branch> | new_statement<ast::unconditional_branch>);
  };

  struct conditional_branch
  {
    static constexpr auto rule = []
    {
      auto alt_condition = dsl::p<dependent_branch>;

      auto introduced = dsl::p<nested_expression> + dsl::p<branch_body> + dsl::opt(alt_condition);
      return keyword::if_ >> introduced;
    }();

    static constexpr auto value =
        lexy::bind(
            lexy::construct<ast::conditional_branch>, lexy::_1, lexy::_2, lexy::_3.or_default())
        | new_statement<ast::conditional_branch>;
  };

  struct let_assignment
  {
    static constexpr auto rule = []
    {
      auto introduced = dsl::p<symbol_identifier> + LEXY_LIT(":") + dsl::p<symbol_identifier>
          + LEXY_LIT("=") + dsl::p<expression>;
      return keyword::variable_ >> introduced;
    }();

    static constexpr auto value =
        lexy::construct<ast::let_assignment> | new_statement<ast::let_assignment>;
  };

  struct expression_statement
  {
    static constexpr auto rule = [] { return keyword::discard_ >> dsl::p<expression>; }();

    static constexpr auto value =
        lexy::construct<ast::expression_statement> | new_statement<ast::expression_statement>;
  };

  struct return_stmt
  {
    static constexpr auto rule = [] { return keyword::return_ >> dsl::p<expression>; }();

    static constexpr auto value = lexy::construct<ast::return_> | new_statement<ast::return_>;
  };

  struct inner_stmt
  {
    struct missing_statement
    {
      static constexpr auto name =
          R"(Expected `val` for an let_assignment, `if` for branching, `discard` for expression statements;)";
    };

    static constexpr auto rule = []
    {
      auto terminator = dsl::terminator(dsl::semicolon).limit(dsl::lit_c<'}'>);
      return terminator(dsl::p<let_assignment> | dsl::p<conditional_branch>
                        | dsl::p<expression_statement> | dsl::p<return_stmt>
                        | dsl::error<missing_statement>);
    }();

    static constexpr auto value = lexy::forward<std::unique_ptr<ast::statement>>;
  };

  struct inner_compound_body
  {
    static constexpr auto rule = []
    { return dsl::curly_bracketed.opt_list(dsl::recurse<struct inner_stmt>); }();

    static constexpr auto value = lexy::as_list<ast::statements>;
  };

  struct outer_compound_body
  {
    static constexpr auto rule = [] { return dsl::curly_bracketed.opt_list(dsl::p<inner_stmt>); }();

    static constexpr auto value = lexy::as_list<ast::statements>;
  };

  /* === Outer Statements === */

  struct function_def
  {
    struct parameter
    {
      static constexpr auto rule = []
      { return dsl::p<symbol_identifier> + LEXY_LIT(":") + dsl::p<type_identifier>; }();
      static constexpr auto value = lexy::construct<ast::parameter>;
    };

    struct parameters
    {
      static constexpr auto rule = []
      { return dsl::round_bracketed.opt_list(dsl::p<parameter>, dsl::trailing_sep(dsl::comma)); }();

      static constexpr auto value = lexy::as_list<std::vector<ast::parameter>> >> lexy::bind(
                                        lexy::construct<ast::parameter_list>,
                                        lexy::_1.or_default());
    };

    static constexpr auto rule = []
    {
      auto introduced = dsl::p<symbol_identifier> + dsl::p<parameters>
          + dsl::opt(LEXY_LIT("->") >> dsl::p<type_identifier>) + dsl::p<outer_compound_body>;
      return keyword::funcdef_ >> introduced;
    }();

    static constexpr auto value = lexy::callback<ast::function_def>(
        [](std::string name,
           ast::parameter_list params,
           std::optional<std::string> rettype,
           ast::statements body)
        {
          auto signature = ast::signature(std::move(name), std::move(params), std::move(rettype));
          return ast::function_def(std::move(signature), std::move(body));
        });
  };

  struct type_def
  {
    struct body
    {
      static constexpr auto rule = [] {
        return dsl::curly_bracketed.opt_list(dsl::p<type_identifier>,
                                             dsl::trailing_sep(dsl::comma));
      }();

      static constexpr auto value = lexy::as_list<ast::type_definition_stmts>;
    };

    static constexpr auto rule = []
    {
      auto introduced = dsl::p<symbol_identifier> + dsl::p<body>;
      return keyword::struct_ >> introduced;
    }();

    static constexpr auto value = lexy::construct<ast::type_definition>;
  };

  struct outer_stmt
  {
    template<typename T>
    static constexpr auto new_outer_statement = new_unique_ptr<T, ast::statement>;

    struct outer_stmt_error
    {
      static constexpr auto name =
          "Expected `def` to define a function, `struct` to define a newtype";
    };

    static constexpr auto rule = []
    { return dsl::p<function_def> | dsl::p<type_def> | dsl::error<outer_stmt_error>; }();
    static constexpr auto value = lexy::callback(new_outer_statement<ast::function_def>,
                                                 new_outer_statement<ast::type_definition>);
  };

  struct mod
  {
    static constexpr auto rule = dsl::list(dsl::p<outer_stmt>, dsl::sep(dsl::newline));

    static constexpr auto value = lexy::as_list<ast::statements> >> lexy::construct<ast::mod>
        | new_unique_ptr<ast::mod, ast::node>;
  };
};

template<typename Input>
struct lexy_frontend
{
  struct lexy_span
  {
    lexy::input_location<Input> begin;
    lexy::input_location<Input> end;
  };

  using span_map =
      std::unordered_map<boost::uuids::uuid, lexy_span, boost::hash<boost::uuids::uuid>>;

  struct lexy_parse_result final : p::parse_metadata
  {
    lexy_parse_result(Input input, span_map span_lookup)
        : m_input {std::move(input)}
        , m_span_lookup {std::move(span_lookup)}
    {
    }

    auto report_error(std::ostream& os,
                      ast::node const& node,
                      p::frontend_error_report report) const -> std::ostream&
    {
      auto span_search = this->m_span_lookup.find(node.uuid);
      if (span_search == this->m_span_lookup.end()) {
        os << "Unable to report error with span! AST Node is does not have an associated span\n";
        os << "Message: " << report.message << "\n";
      } else {
        auto const& span = span_search->second;
        static constexpr auto opts = lexy::visualization_options {} | lexy::visualize_fancy;

        lexy_ext::diagnostic_writer(this->m_input, opts)
            .write_annotation(std::ostream_iterator<std::string_view::value_type>(os),
                              lexy_ext::annotation_kind::primary,
                              span.begin,
                              span.end.position(),
                              [&](auto& out, lexy::visualization_options)
                              { return lexy::_detail::write_str(out, report.message.c_str()); });
      }

      return os;
    }

  private:
    Input m_input;
    span_map m_span_lookup;
  };

  struct lexy_state
  {
    explicit lexy_state(Input& input_)
        : input {input_}
    {
    }

    Input& input;
    span_map span_lookup;
  };

  static auto parse(std::string_view code) -> p::frontend_parse_result
  {
    using entrypoint = top_level<typename lexy_grammar<Input>::mod>;
    return lexy_frontend<Input>::parse_entrypoint<entrypoint>(code);
  }

  static auto parse_expression(std::string_view code) -> p::frontend_parse_result
  {
    using entrypoint = top_level<typename lexy_grammar<Input>::expression>;
    return lexy_frontend<Input>::parse_entrypoint<entrypoint>(code);
  }

  static auto parse_statement(std::string_view code) -> p::frontend_parse_result
  {
    using entrypoint = top_level<typename lexy_grammar<Input>::inner_stmt>;
    return lexy_frontend<Input>::parse_entrypoint<entrypoint>(code);
  }

private:
  template<typename Entrypoint>
  static auto parse_entrypoint(std::string_view code) -> p::frontend_parse_result
  {
    auto input = Input {code};
    auto state = lexy_state {input};

    std::string error;
    auto error_handling = lexy_ext::report_error.to(std::back_insert_iterator(error));

    if (auto tree = lexy::parse<Entrypoint>(input, state, error_handling); tree.is_success()) {
      auto lexy_pr =
          std::make_unique<lexy_parse_result>(std::move(input), std::move(state.span_lookup));
      auto ast = std::move(tree).value();

      return p::frontend_parse_result(std::move(lexy_pr), std::move(ast));
    }

    return p::frontend_parse_result(std::move(error));
  }
};

namespace bython::parser
{

auto lexy_code_frontend::parse(std::string_view code) -> frontend_parse_result
{
  using parser = lexy_frontend<lexy::string_input<>>;
  return parser::parse(code);
}

auto lexy_code_frontend::parse_expression(std::string_view code) -> frontend_parse_result
{
  using parser = lexy_frontend<lexy::string_input<>>;
  return parser::parse_expression(code);
}

auto lexy_code_frontend::parse_statement(std::string_view code) -> frontend_parse_result
{
  using parser = lexy_frontend<lexy::string_input<>>;
  return parser::parse_statement(code);
}
}  // namespace bython::parser
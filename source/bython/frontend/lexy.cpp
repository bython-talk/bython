#include <concepts>
#include <iostream>
#include <iterator>
#include <type_traits>

#include "lexy.hpp"

#include <boost/container_hash/hash.hpp>  // uuid hasher
#include <boost/uuid/uuid.hpp>  // uuid class
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/error.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy/input_location.hpp>
#include <lexy_ext/report_error.hpp>

#include "bython/ast.hpp"
#include "lexy/top_level_grammar.hpp"

namespace dsl = lexy::dsl;

namespace ast = bython::ast;
namespace p = bython::parser;

template<class T>
struct is_unique_ptr : std::false_type
{
};

template<class T, class D>
struct is_unique_ptr<std::unique_ptr<T, D>> : std::true_type
{
};

template<typename Input>
struct lexy_grammar
{
  template<typename Production, typename Value>
  struct with_span : lexy::transparent_production
  {
    static constexpr auto rule = dsl::position + dsl::p<Production> + dsl::position;

    static constexpr auto value = lexy::callback_with_state<Value>(
        [](auto& state, auto& startptr, Value node, auto& endptr)
        {
          auto begin = lexy::get_input_location(state.input, startptr);
          auto end = lexy::get_input_location(state.input, endptr);

          using LexySpan = decltype(state.span_lookup)::mapped_type;
          auto node_span = LexySpan {.begin = begin, .end = end};

          if constexpr (is_unique_ptr<std::remove_cvref<Value>>::value) {
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

  struct nested_expression : lexy::transparent_production
  {
    static constexpr auto rule = dsl::recurse<struct expr_prod>;
    static constexpr auto value = lexy::forward<ast::expression_ptr>;
  };

  struct integer : lexy::token_production
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule =
          // Hexadecimal: 0x0F
          LEXY_LIT("0x") >> dsl::integer<int64_t, dsl::hex> |
          // Explicit Decimal: 0d09
          LEXY_LIT("0d") >> dsl::integer<int64_t, dsl::decimal> |
          // Explicit Octal: 0o07
          LEXY_LIT("0o") >> dsl::integer<int64_t, dsl::octal> |
          // Explicit Binary: 0b01
          LEXY_LIT("0b") >> dsl::integer<int64_t, dsl::binary> |
          // Default; standard decimal with an optional sign
          dsl::peek(dsl::lit_c<'-'> / dsl::lit_c<'+'> / dsl::digit<dsl::decimal>)
              >> dsl::sign + dsl::integer<int64_t, dsl::decimal>;
      static constexpr auto value = lexy::as_integer<int64_t> | lexy::construct<ast::integer>;
    };

    static constexpr auto rule = dsl::p<with_span<inner, ast::integer>>;
    static constexpr auto value = new_expression<ast::integer>;
  };

  struct argument_list
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule =
          dsl::round_bracketed.opt_list(dsl::p<nested_expression>, dsl::trailing_sep(dsl::comma));
      static constexpr auto value =
          lexy::as_list<ast::expressions> | lexy::construct<ast::argument_list>;
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
    static constexpr auto as = dsl::op<ast::binop_tag::as>(keyword::as_);

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

    struct as_conversion : dsl::infix_op_left
    {
      static constexpr auto op = binary_operators::as;
      using operand = dsl::atom;
    };

    struct math_power : dsl::infix_op_right
    {
      static constexpr auto op = binary_operators::pow;
      using operand = as_conversion;
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

    struct comparison : dsl::infix_op_list
    {
      static constexpr auto op = binary_operators::neq / binary_operators::eq
          / binary_operators::grt / binary_operators::geq / binary_operators::leq
          / binary_operators::lsr;
      using operand = logical_or;
    };

    using operation = comparison;

    static constexpr auto value =
        lexy::fold_inplace<std::unique_ptr<ast::comparison>>(
            []
            {
              auto empty_comp = ast::comparison {ast::expressions {},
                                                 std::vector<ast::comparison_operator_tag> {}};
              return std::make_unique<ast::comparison>(std::move(empty_comp));
            },
            [](std::unique_ptr<ast::comparison>& comparison, ast::expression_ptr expr)
            { comparison->add_operand(std::move(expr)); },
            [](std::unique_ptr<ast::comparison>& comparison, ast::comparison_operator_tag cmp)
            { comparison->add_operator(std::move(cmp)); })
        >> lexy::callback(new_expression<ast::call>,
                          new_expression<ast::variable>,
                          new_expression<ast::integer>,
                          new_expression<ast::binary_operation>,
                          new_expression<ast::unary_operation>,
                          new_expression<ast::comparison>,
                          lexy::forward<ast::expression_ptr>);
  };

  struct expression
  {
    static constexpr auto rule = dsl::p<with_span<expr_prod, ast::expression_ptr>>;
    static constexpr auto value = lexy::forward<ast::expression_ptr>;
  };

  struct var
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule =
          dsl::identifier(dsl::ascii::alpha_underscore, dsl::ascii::alpha_digit_underscore);
      static constexpr auto value = lexy::as_string<std::string> | lexy::construct<ast::variable>;
    };

    static constexpr auto rule = dsl::p<with_span<inner, ast::variable>>;
    static constexpr auto value = lexy::new_<ast::variable, ast::expression_ptr>;
  };

  struct vars
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule = dsl::list(dsl::p<var>, dsl::sep(dsl::comma));
      static constexpr auto value =
          lexy::as_list<std::vector<ast::expression_ptr>> >> lexy::construct<ast::expr_mod>;
    };

    static constexpr auto rule = dsl::p<with_span<inner, ast::expr_mod>>;
    static constexpr auto value = lexy::new_<ast::expr_mod, std::unique_ptr<ast::node>>;
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
    lexy_parse_result(Input input_, span_map span_lookup_)
        : input {std::move(input_)}
        , span_lookup {std::move(span_lookup_)}
    {
    }

    Input input;
    span_map span_lookup;
  };

  struct lexy_state
  {
    lexy_state(Input& input_)
        : input {input_}
    {
    }

    Input& input;
    span_map span_lookup;
  };

  static auto parse(std::string_view code) -> p::frontend_parse_result
  {
    using entrypoint = lexy_grammar<Input>::vars;
    return lexy_frontend<Input>::parse_entrypoint<entrypoint>(code);
  }

  static auto parse_expression(std::string_view code) -> p::frontend_parse_result
  {
    using entrypoint = lexy_grammar<Input>::expression;
    return lexy_frontend<Input>::parse_entrypoint<entrypoint>(code);
  }

  static auto parse_statement(std::string_view code) -> p::frontend_parse_result
  {
    using entrypoint = lexy_grammar<Input>::vars;
    return lexy_frontend<Input>::parse_entrypoint<entrypoint>(code);
  }

  static auto report_error(p::parse_metadata const& tree,
                           ast::node const& node,
                           p::frontend_error_report report) -> void
  {
    auto lexy_parse_tree = dynamic_cast<lexy_parse_result const*>(&tree);
    if (!lexy_parse_tree) {
      std::cerr << "Cannot report lexy error without lexy's input\n";
      return;
    }

    auto span_search = lexy_parse_tree->span_lookup.find(node.uuid);
    if (span_search == lexy_parse_tree->span_lookup.end()) {
      std::cerr << "Unable to report error! AST Node is known\n";
      return;
    }
    auto const& span = span_search->second;

    static constexpr auto opts = lexy::visualization_options {} | lexy::visualize_fancy;
    lexy_ext::diagnostic_writer(lexy_parse_tree->input, opts)
        .write_annotation(std::ostream_iterator<std::string_view::value_type>(std::cerr),
                          lexy_ext::annotation_kind::primary,
                          span.begin,
                          span.end.position(),
                          [&](auto& out, lexy::visualization_options)
                          { return lexy::_detail::write_str(out, report.message.c_str()); });
  }

private:
  template<typename Entrypoint>
  static auto parse_entrypoint(std::string_view code) -> p::frontend_parse_result
  {
    auto input = Input {code};
    auto state = lexy_state {input};

    std::string error;
    auto error_handling = lexy_ext::report_error.to(std::back_insert_iterator(error));

    if (auto tree = lexy::parse<Entrypoint>(input, state, error_handling); tree.has_value()) {
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

auto lexy_code_frontend::report_error(p::parse_metadata const& tree,
                                      ast::node const& node,
                                      p::frontend_error_report report) const -> void
{
  using parser = lexy_frontend<lexy::string_input<>>;
  parser::report_error(tree, node, std::move(report));
}
}  // namespace bython::parser
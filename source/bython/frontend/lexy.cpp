#include <concepts>
#include <iostream>
#include <iterator>
#include <type_traits>

#include "lexy.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
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
struct lexy_parse_result final : p::parse_tree
{
  lexy_parse_result(Input input_)
      : input {std::move(input_)}
  {
  }

  Input input;
};

template<typename Input>
struct lexy_state
{
  using lexy_span = p::lexy_frontend<Input>::lexy_span;

  lexy_state(Input& input_)
      : input {input_}
      , spans {}
  {
  }

  Input& input;
  std::unordered_map<boost::uuids::uuid, lexy_span, boost::hash<boost::uuids::uuid>> spans;
};

template<typename Input>
struct lexy_grammar
{
  using lexy_span = p::lexy_frontend<Input>::lexy_span;

  template<typename Production, typename Value>
  struct with_span : lexy::transparent_production
  {
    static constexpr auto rule = dsl::position + dsl::p<Production> + dsl::position;

    static constexpr auto value = lexy::callback_with_state<Value>(
        [](auto& state, auto& startptr, Value node, auto& endptr)
        {
          auto begin = lexy::get_input_location(state.input, startptr);
          auto end = lexy::get_input_location(state.input, endptr);

          auto node_span = lexy_span {.begin = begin, .end = end};

          if constexpr (is_unique_ptr<std::remove_cvref<Value>>::value) {
            state.spans.insert({node->uuid, std::move(node_span)});
          } else {
            state.spans.insert({node.uuid, std::move(node_span)});
          }
          return node;
        },
        [](auto&,  // startptr
           Value&& node,
           auto&  // endptr
        ) { return node; });
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
    static constexpr auto value = lexy::new_<ast::variable, std::unique_ptr<ast::expression>>;
  };

  struct vars
  {
    struct inner : lexy::transparent_production
    {
      static constexpr auto rule = dsl::list(dsl::p<var>, dsl::sep(dsl::comma));
      static constexpr auto value =
          lexy::as_list<std::vector<std::unique_ptr<ast::expression>>> >> lexy::construct<
              ast::expr_mod>;
    };

    static constexpr auto rule = dsl::p<with_span<inner, ast::expr_mod>>;
    static constexpr auto value = lexy::new_<ast::expr_mod, std::unique_ptr<ast::node>>;
  };
};

namespace bython::parser
{

template<typename Input, typename Entrypoint>
auto parse_entrypoint(std::string_view code,
                      std::unordered_map<boost::uuids::uuid,
                                         typename p::lexy_frontend<Input>::lexy_span,
                                         boost::hash<boost::uuids::uuid>>* span_lookup)
    -> frontend_parse_result
{
  auto input = Input {code};
  auto state = lexy_state {input};

  std::string error;
  auto error_handling = lexy_ext::report_error.to(std::back_insert_iterator(error));

  if (auto tree = lexy::parse<Entrypoint>(input, state, error_handling); tree.has_value()) {
    *span_lookup = std::move(state.spans);

    auto module = std::move(tree).value();
    std::unique_ptr<parse_tree> lexy_pr =
        std::make_unique<lexy_parse_result<Input>>(std::move(input));

    return frontend_parse_result(std::move(lexy_pr), std::move(module));
  }

  return frontend_parse_result(std::move(error));
}

auto lexy_code_frontend::parse(std::string_view code) -> frontend_parse_result
{
  return parse_entrypoint<lexy::string_input<>, lexy_grammar<lexy::string_input<>>::vars>(code, &this->span_lookup);
}

auto lexy_code_frontend::parse_expression(std::string_view code) -> frontend_parse_result
{
  return parse_entrypoint<lexy::string_input<>, lexy_grammar<lexy::string_input<>>::vars>(code, &this->span_lookup);
}

auto lexy_code_frontend::parse_statement(std::string_view code) -> frontend_parse_result
{
  return parse_entrypoint<lexy::string_input<>, lexy_grammar<lexy::string_input<>>::vars>(code, &this->span_lookup);
}

auto lexy_code_frontend::report_error(parser::parse_tree const& tree,
                                      ast::node const& node,
                                      frontend_error_report report) const -> void
{
  using pr = lexy_parse_result<lexy::string_input<>>;

  auto lexy_parse_tree = dynamic_cast<pr const*>(&tree);
  if (!lexy_parse_tree) {
    std::cerr << "Cannot report lexy error without lexy's input\n";
  }

  auto span_search = this->span_lookup.find(node.uuid);
  if (span_search == this->span_lookup.end()) {
    std::cerr << "Unable to report error! AST Node is known\n";
    return;
  }
  auto span = span_search->second;

  lexy_ext::diagnostic_writer(lexy_parse_tree->input)
      .write_annotation(std::ostream_iterator<std::string_view::value_type>(std::cerr),
                        lexy_ext::annotation_kind::primary,
                        span.begin,
                        span.end.position(),
                        [&](auto& out, lexy::visualization_options)
                        { return lexy::_detail::write_str(out, report.message.c_str()); });
}
}  // namespace bython::parser
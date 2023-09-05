#include <concepts>
#include <iostream>
#include <iterator>
#include <type_traits>

#include "lexy.hpp"

#include <boost/container_hash/hash.hpp>  // uuid hasher
#include <boost/uuid/uuid.hpp>  // uuid class
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
    using entrypoint = lexy_grammar<Input>::vars;
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
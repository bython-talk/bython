#pragma once

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include "bython/ast.hpp"

namespace bython::grammar
{

namespace dsl = lexy::dsl;

template<typename Production, typename Value>
struct with_span : lexy::transparent_production
{
  using value_type = lexy_wrapped<Value>;

  static constexpr auto rule = dsl::position + dsl::p<Production> + dsl::position;

  static constexpr auto value = lexy::callback_with_state<value_type>(
      [](auto& state, auto& startptr, Value&& node, auto& endptr)
      {
        auto begin = lexy::get_input_location(state.input, startptr);
        auto end = lexy::get_input_location(state.input, endptr);

        auto node_span = lexy_span {.begin = begin, .end = end};
        return value_type {.node = std::move(node), .span = std::move(node_span)};
      },
      [](auto&,  // startptr
         Value&& node,
         auto&  // endptr
      ) { return value_type {.node = std::move(node), .span = std::nullopt}; });
};



}  // namespace bython::grammar
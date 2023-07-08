#pragma once

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

#include "bython/ast/module.hpp"
#include "outer_stmt.hpp"

namespace bython::grammar
{

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
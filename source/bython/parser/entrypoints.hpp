#pragma once

#include <lexy/action/parse.hpp>
#include <lexy_ext/report_error.hpp>

#include "../ast/compound.hpp"
#include "../ast/expression.hpp"
#include "../ast/module.hpp"
#include "../ast/statement.hpp"

namespace bython::parser
{
using error_callback = std::decay_t<decltype(lexy_ext::report_error)>;

auto module(std::string_view code, error_callback report_error)
    -> lexy::parse_result<ast::mod, error_callback>;

auto expression(std::string_view code, error_callback report_error)
    -> lexy::parse_result<std::unique_ptr<ast::expression>, error_callback>;

auto statement(std::string_view code, error_callback report_error)
    -> lexy::parse_result<std::unique_ptr<ast::statement>, error_callback>;

}  // namespace bython::parser
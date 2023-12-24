#pragma once

#include <optional>
#include <unordered_map>
#include <variant>

#include <lexy/input/string_input.hpp>
#include <lexy/input_location.hpp>

#include "frontend.hpp"

namespace bython::parser
{

struct lexy_code_frontend final : frontend
{
  virtual auto parse(std::string_view code) -> frontend_parse_result;
  virtual auto parse_expression(std::string_view code) -> frontend_parse_result;
  virtual auto parse_statement(std::string_view code) -> frontend_parse_result;
};

/*
class lexy_file_frontend final : public frontend
{
  virtual auto parse(std::string_view) -> std::variant<std::unique_ptr<ast::node>, std::string> = 0;
  virtual auto report_error(ast::node const& node, frontend_error_report report) const -> void;
};

class lexy_shell_frontend final : public frontend
{
  virtual auto parse(std::string_view) -> std::variant<std::unique_ptr<ast::node>, std::string> = 0;
  virtual auto report_error(ast::node const& node, frontend_error_report report) const -> void;
};
*/
}  // namespace bython::parser
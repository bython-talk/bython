#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <optional>
#include <ostream>
#include <variant>

#include "bython/ast.hpp"

namespace bython::parser
{

struct frontend_error_report
{
  std::string message;
};

struct frontend_error_reporter
{
  auto message(std::string explicit_message) -> frontend_error_reporter;
  auto build() const -> frontend_error_report;

  std::string msg;
};

struct parse_metadata
{
  virtual ~parse_metadata() = default;
  auto report_error(ast::node const& node, frontend_error_report report) const -> std::ostream&;
  virtual auto report_error(std::ostream& os,
                            ast::node const& node,
                            frontend_error_report report) const -> std::ostream& = 0;
};

struct frontend_parse_result
{
  frontend_parse_result() = delete;

  frontend_parse_result(std::unique_ptr<parse_metadata> tree, std::unique_ptr<ast::node> ast);
  explicit frontend_parse_result(std::string error);

  auto has_value() const -> bool;
  auto has_error() const -> bool;

  using value_type = std::tuple<std::unique_ptr<parse_metadata>, std::unique_ptr<ast::node>>;

  auto value() && -> value_type;
  auto error() && -> std::string;

private:
  std::variant<value_type, std::string> result_;
};

struct frontend
{
  frontend() = default;
  virtual ~frontend() = default;

  // Disable copying of derived types
  frontend(frontend const&) = delete;

  virtual auto parse(std::string_view) -> frontend_parse_result = 0;
  virtual auto parse_expression(std::string_view) -> frontend_parse_result = 0;
  virtual auto parse_statement(std::string_view) -> frontend_parse_result = 0;
};

}  // namespace bython::parser
#include <iostream>
#include <variant>

#include "frontend.hpp"

namespace bython::parser
{

frontend_parse_result::frontend_parse_result(std::unique_ptr<parse_metadata> tree,
                                             std::unique_ptr<ast::node> ast)
    : result_ {std::make_tuple(std::move(tree), std::move(ast))}
{
}

frontend_parse_result::frontend_parse_result(std::string error)
    : result_ {std::move(error)}
{
}

auto frontend_parse_result::has_value() const -> bool
{
  return std::holds_alternative<frontend_parse_result::value_type>(this->result_);
}

auto frontend_parse_result::has_error() const -> bool
{
  return std::holds_alternative<std::string>(this->result_);
}

auto frontend_parse_result::value() && -> value_type
{
  return std::get<frontend_parse_result::value_type>(std::move(this->result_));
}

auto frontend_parse_result::error() && -> std::string
{
  return std::get<std::string>(this->result_);
}

auto frontend_error_reporter::message(std::string explicit_message) -> frontend_error_reporter
{
  this->msg = std::move(explicit_message);
  return *this;
}

auto frontend_error_reporter::build() const -> frontend_error_report
{
  return frontend_error_report {.message = this->msg};
}

auto parse_metadata::report_error(ast::node const& node, frontend_error_report report) const -> std::ostream& {
  return this->report_error(std::cerr, node, std::move(report));
}

}  // namespace bython::parser
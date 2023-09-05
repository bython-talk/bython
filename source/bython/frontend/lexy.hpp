#pragma once

#include <optional>
#include <unordered_map>
#include <variant>

#include <lexy/input/string_input.hpp>
#include <lexy/input_location.hpp>

#include "frontend.hpp"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/container_hash/hash.hpp>            // uuid hasher

namespace bython::parser
{



template<typename Input>
struct lexy_frontend : frontend
{
  struct lexy_span
  {
    lexy::input_location<Input> begin;
    lexy::input_location<Input> end;
  };

protected:
  std::unordered_map<boost::uuids::uuid, lexy_span, boost::hash<boost::uuids::uuid>> span_lookup;
};

struct lexy_code_frontend final : lexy_frontend<lexy::string_input<>>
{
  virtual auto parse(std::string_view code) -> frontend_parse_result;
  virtual auto parse_expression(std::string_view code) -> frontend_parse_result;
  virtual auto parse_statement(std::string_view code) -> frontend_parse_result;

  virtual auto report_error(parser::parse_tree const& tree,
                            ast::node const& node,
                            frontend_error_report report) const -> void;
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
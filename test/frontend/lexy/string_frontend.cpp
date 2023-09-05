#include <catch2/catch_test_macros.hpp>

#include "bython/frontend/lexy.hpp"

namespace p = bython::parser;
namespace ast = bython::ast;

using node_ptr = std::unique_ptr<ast::node>;

TEST_CASE("Lexy Frontend")
{
  auto parser = p::lexy_code_frontend {};
  auto parse_result = parser.parse("undecl_iden");
  REQUIRE(parse_result.has_value());

  auto [parse_tree, ast] = std::move(parse_result).value();
  parser.report_error(*parse_tree,
                      *ast,
                      p::frontend_error_reporter {}
                          .message("undeclared identifier found; did you mean: undeclIdent")
                          .build());
}
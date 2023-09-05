#include <catch2/catch_test_macros.hpp>

#include "bython/ast.hpp"
#include "bython/frontend/lexy.hpp"

namespace p = bython::parser;
namespace ast = bython::ast;

using node_ptr = std::unique_ptr<ast::node>;

TEST_CASE("Lexy Frontend")
{
  auto parser = p::lexy_code_frontend {};
  auto parse_result = parser.parse("undecl_iden,DeclIdent");
  REQUIRE(parse_result.has_value());

  auto [parse_metadata, ast] = std::move(parse_result).value();
  auto mod = dynamic_cast<ast::expr_mod*>(&*ast);
  REQUIRE(mod != nullptr);

  parser.report_error(*parse_metadata,
                      *mod->body[0],
                      p::frontend_error_reporter {}
                          .message("undeclared identifier found; did you mean: undeclIdent")
                          .build());

  parser.report_error(*parse_metadata,
                      *mod->body[1],
                      p::frontend_error_reporter {}
                          .message("undeclared identifier found; did you mean: DeclIdentifier")
                          .build());

  parser.report_error(*parse_metadata,
                      *mod,
                      p::frontend_error_reporter {}
                          .message("Unknown notation; Please clarify with parentheses")
                          .build());
}
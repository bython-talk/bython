#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bases.hpp"
#include "expression.hpp"

namespace bython::ast
{
struct statement : node
{
};

using statements = std::vector<std::unique_ptr<statement>>;

struct type_definition_stmt
{
  explicit type_definition_stmt(std::string identifier_)
      : identifier {std::move(identifier_)}
  {
  }

  std::string identifier;

  auto tag() const -> ast::tag;
};
using type_definition_stmts = std::vector<type_definition_stmt>;

struct type_definition final : statement
{
  type_definition(std::string identifier_, type_definition_stmts body_);

  std::string identifier;
  type_definition_stmts body;

  auto tag() const -> ast::tag;
};

struct assignment final : statement
{
  assignment(std::string lhs_, std::string hint_, std::unique_ptr<expression> rhs_);

  std::string lhs;
  std::string hint;
  std::unique_ptr<expression> rhs;

  auto tag() const -> ast::tag;
};

struct expression_statement final : statement
{
  expression_statement(std::unique_ptr<expression> discarded_);

  std::unique_ptr<expression> discarded;

  auto tag() const -> ast::tag;
};

}  // namespace bython::ast
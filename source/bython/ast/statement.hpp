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

struct for_ final : statement
{
  explicit for_(statements body_);

  statements body;

  auto tag() const -> ast::tag;
};

struct while_ final : statement
{
  explicit while_(statements body_);

  statements body;

  auto tag() const -> ast::tag;
};

struct conditional_branch final : statement
{
  conditional_branch(std::unique_ptr<expression> condition_, statements body_);
  conditional_branch(std::unique_ptr<expression> condition_,
                     statements body_,
                     std::unique_ptr<statement> orelse_);

  std::unique_ptr<node> condition;
  std::unique_ptr<node> orelse;

  statements body;

  auto tag() const -> ast::tag;
};

struct unconditional_branch final : statement
{
  explicit unconditional_branch(statements body_);

  statements body;

  auto tag() const -> ast::tag;
};

struct parameter
{
  std::string name;
};

using parameters = std::vector<parameter>;

struct function_def final : statement
{
  function_def(std::string name_, std::vector<parameter> parameters_, statements body_);

  std::string name;
  std::vector<parameter> parameters;

  statements body;

  auto tag() const -> ast::tag;
};

}  // namespace bython::ast
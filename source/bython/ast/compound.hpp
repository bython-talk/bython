#pragma once

#include <vector>

#include "bases.hpp"
#include "statement.hpp"
#include "tags.hpp"

namespace bython::ast
{

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

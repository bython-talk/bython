#pragma once

#include <bython/ast/expression.hpp>

#include "bases.hpp"

namespace bython::matching
{
struct expression : matcher
{
};

struct unary_operation final : matching::expression
{
  unary_operation(ast::unary_operator op_,
                  std::unique_ptr<matching::expression> rhs_matcher_)
      : op {op_}
      , rhs_matcher {std::move(rhs_matcher_)}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  ast::unary_operator op;
  std::unique_ptr<matching::expression> rhs_matcher;
};

struct binary_operation final : matching::expression
{
  binary_operation(std::unique_ptr<matching::expression> lhs_matcher_,
                   ast::binary_operator op_,
                   std::unique_ptr<matching::expression> rhs_matcher_)
      : lhs_matcher {std::move(lhs_matcher_)}
      , op {op_}
      , rhs_matcher {std::move(rhs_matcher_)}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  std::unique_ptr<matching::expression> lhs_matcher;
  ast::binary_operator op;
  std::unique_ptr<matching::expression> rhs_matcher;
};

struct comparison final : matching::expression
{
  explicit comparison(std::unique_ptr<matching::expression> init)
  {
    this->operands.emplace_back(std::move(init));
  }

  auto chain(ast::comparison_operator op,
             std::unique_ptr<matching::expression> against) -> comparison;

  auto matches(ast::node const& ast) const -> bool override;

  std::vector<std::unique_ptr<matching::expression>> operands;
  std::vector<ast::comparison_operator> ops;
};

struct integer final : matching::expression
{
  explicit integer(int64_t value_)
      : value {value_}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  int64_t value;
};

struct variable final : matching::expression
{
  explicit variable(std::string identifier_)
      : identifier {std::move(identifier_)}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  std::string identifier;
};

}  // namespace bython::matching
#pragma once

#include <bython/ast/expression.hpp>

#include "bases.hpp"
#include "operators.hpp"

namespace bython::matching
{
struct expression : matcher
{
};

struct unary_operation final : matching::expression
{
  unary_operation(std::unique_ptr<matching::matcher> op_,
                  std::unique_ptr<matching::matcher> rhs_matcher_)
      : op_matcher {std::move(op_)}
      , rhs_matcher {std::move(rhs_matcher_)}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  std::unique_ptr<matching::matcher> op_matcher;
  std::unique_ptr<matching::matcher> rhs_matcher;
};

struct binary_operation final : matching::expression
{
  binary_operation(std::unique_ptr<matching::matcher> lhs_matcher_,
                   std::unique_ptr<matching::matcher> op_,
                   std::unique_ptr<matching::matcher> rhs_matcher_)
      : lhs_matcher {std::move(lhs_matcher_)}
      , op {std::move(op_)}
      , rhs_matcher {std::move(rhs_matcher_)}
  {
  }

  auto matches(ast::node const& ast) const -> bool override;

  std::unique_ptr<matching::matcher> lhs_matcher;
  std::unique_ptr<matching::matcher> op;
  std::unique_ptr<matching::matcher> rhs_matcher;
};

struct comparison final : matching::expression
{
  explicit comparison(std::unique_ptr<matching::matcher> init)
  {
    this->operands.emplace_back(std::move(init));
  }

  auto chain(std::unique_ptr<matching::matcher> op,
             std::unique_ptr<matching::matcher> against) -> comparison;

  auto matches(ast::node const& ast) const -> bool override;

  std::vector<std::unique_ptr<matching::matcher>> operands;
  std::vector<std::unique_ptr<matching::matcher>> ops;
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
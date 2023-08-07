#include "tags.hpp"

namespace bython::ast
{
auto tag::is_expression() const -> bool
{
  return std::underlying_type_t<ranges>(ranges::expression) <= this->tag_
      && this->tag_ <= std::underlying_type_t<ranges>(ranges::statement);
}

auto tag::is_statement() const -> bool
{
  return std::underlying_type_t<ranges>(ranges::statement) <= this->tag_
      && this->tag_ <= std::underlying_type_t<ranges>(ranges::misc);
}

auto tag::is_misc() const -> bool
{
  return std::underlying_type_t<ranges>(ranges::misc) <= this->tag_;
      //&& this->tag_ <= std::underlying_type_t<ranges>(ranges::misc);
}

auto tag::unwrap() const -> std::uint32_t
{
  return this->tag_;
}
}  // namespace bython::ast
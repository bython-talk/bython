#pragma once

#include <string>

#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>

namespace bython::grammar
{
namespace dsl = lexy::dsl;

namespace internal
{
struct _identifier
{
  static constexpr auto rule = []
  {
    auto head = dsl::ascii::alpha_underscore;
    auto tail = dsl::ascii::alpha_digit_underscore;
    return dsl::identifier(head, tail);
  }();
};
}  // namespace internal

namespace keyword
{
static constexpr auto funcdef_ =
    LEXY_KEYWORD("def", internal::_identifier::rule);
static constexpr auto return_ =
    LEXY_KEYWORD("return", internal::_identifier::rule);
static constexpr auto variable_ =
    LEXY_KEYWORD("val", internal::_identifier::rule);
static constexpr auto struct_ =
    LEXY_KEYWORD("struct", internal::_identifier::rule);
}  // namespace keyword

struct symbol_identifier
{
  static constexpr auto rule =
      internal::_identifier::rule.reserve(keyword::funcdef_,
                                          keyword::return_,
                                          keyword::variable_,
                                          keyword::struct_);
  static constexpr auto value = lexy::as_string<std::string>;
};

struct type_identifier
{
  static constexpr auto rule =
      internal::_identifier::rule.reserve(keyword::funcdef_,
                                          keyword::return_,
                                          keyword::variable_,
                                          keyword::struct_);
  static constexpr auto value = lexy::as_string<std::string>;
};

template<typename T, typename U>
static constexpr auto new_unique_ptr = lexy::new_<T, std::unique_ptr<U>>;

};  // namespace bython::grammar

#include <algorithm>
#include <cstddef>

#include "builtin.hpp"

namespace bython::type_system
{
auto type::operator!=(type const& other) const -> bool
{
  return !(*this == other);
}

auto void_::operator==(type const& other) const -> bool
{
  return other.tag() == type_tag::void_;
}

auto void_::tag() const -> type_tag
{
  return type_tag::void_;
}

uint::uint(unsigned width_)
    : width {width_}
{
}

auto uint::tag() const -> type_tag
{
  return type_tag::uint;
}

auto uint::operator==(type const& other) const -> bool
{
  auto const* other_uint = dynamic_cast<uint const*>(&other);
  return other_uint != nullptr && this->width == other_uint->width;
}

sint::sint(unsigned width_)
    : width {width_}
{
}

auto sint::tag() const -> type_tag
{
  return type_tag::sint;
}

auto sint::operator==(type const& other) const -> bool
{
  auto const* other_sint = dynamic_cast<sint const*>(&other);
  return other_sint != nullptr && this->width == other_sint->width;
}

auto single_fp::tag() const -> type_tag
{
  return type_tag::single_fp;
}

auto single_fp::operator==(type const& other) const -> bool
{
  auto const* other_single = dynamic_cast<single_fp const*>(&other);
  return other_single != nullptr;
}

auto double_fp::tag() const -> type_tag
{
  return type_tag::double_fp;
}

auto double_fp::operator==(type const& other) const -> bool
{
  auto const* other_double_fp = dynamic_cast<double_fp const*>(&other);
  return other_double_fp != nullptr;
}

auto boolean::tag() const -> type_tag
{
  return type_tag::boolean;
}

auto boolean::operator==(type const& other) const -> bool
{
  auto const* other_boolean = dynamic_cast<boolean const*>(&other);
  return other_boolean != nullptr;
}

function_signature::function_signature(std::vector<type*> parameters_, type* rettype_)
    : parameters {std::move(parameters_)}
    , rettype {rettype_}
{
}

auto function_signature::operator==(type const& other) const -> bool
{
  auto const* other_f = dynamic_cast<function_signature const*>(&other);
  return other_f != nullptr && this->parameters == other_f->parameters
      && this->rettype == other_f->rettype;
}

auto function_signature::tag() const -> type_tag
{
  return type_tag::function;
}

/// Builtin functions
function::function(function_signature signature_)
    : signature {std::move(signature_)}
{
}

auto function::operator==(function const& other) const -> bool
{
  return this->tag() == other.tag();
}

auto function::operator!=(function const& other) const -> bool
{
  return this->tag() != other.tag();
}

auto put_i64::tag() const -> function_tag
{
  return function_tag::put_i64;
}

auto put_u64::tag() const -> function_tag
{
  return function_tag::put_u64;
}

auto put_f32::tag() const -> function_tag
{
  return function_tag::put_f32;
}

}  // namespace bython::type_system
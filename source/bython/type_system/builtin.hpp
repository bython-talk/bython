#pragma once

#include <optional>
#include <vector>
namespace bython::type_system
{
enum class type_tag
{
  void_,
  boolean,
  uint,
  sint,
  single_fp,
  double_fp,
  function,
};

struct type
{
  type() = default;
  virtual ~type() = default;

  type(const type&) = default;
  auto operator=(const type&) -> type& = default;

  type(type&&) = delete;
  auto operator=(type&&) -> type& = delete;

  virtual auto operator==(type const& other) const -> bool = 0;
  virtual auto operator!=(type const& other) const -> bool final;

  virtual auto tag() const -> type_tag = 0;
};

struct void_ final : type
{
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
};

struct uint final : type
{
  uint(unsigned width);

  unsigned width;
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
};

struct sint final : type
{
  sint(unsigned width);

  unsigned width;
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
};

struct single_fp final : type
{
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
};

struct double_fp final : type
{
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
};

struct boolean final : type
{
  auto operator==(type const& other) const -> bool;

  auto tag() const -> type_tag;
};

struct func_sig final : type
{
  func_sig(std::vector<type*> parameters, type* rettype);

  std::vector<type*> parameters;
  type* rettype;

  auto operator==(type const& other) const -> bool;
  auto tag() const -> type_tag;
};

}  // namespace bython::type_system
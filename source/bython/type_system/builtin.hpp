#pragma once

#include <cstdint>
#include <optional>
#include <vector>
namespace bython::type_system
{

/// Builtin types
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

  type(const type&) = delete;
  auto operator=(const type&) -> type& = delete;

  type(type&&) = default;
  auto operator=(type&&) -> type& = default;

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

struct function_signature final : type
{
  function_signature(std::vector<type*> parameters, type* rettype);

  std::vector<type*> parameters;
  type* rettype;

  auto operator==(type const& other) const -> bool;
  auto tag() const -> type_tag;
};

/// Builtin functions
enum class function_tag : uint8_t
{
  // Various IO functions
  put_i64,
  put_u64,

  put_f32,
  put_f64,
};

struct function
{
  function() = delete;
  explicit function(function_signature signature);

  virtual ~function() = default;

  function(function const&) = delete;
  auto operator=(function const&) = delete;

  function(function&&) = default;
  auto operator=(function&&) -> function& = delete;

  auto operator==(function const& other) const -> bool;
  auto operator!=(function const& other) const -> bool;

  virtual auto tag() const -> function_tag = 0;

  function_signature signature;
};

/// IO functions
struct put_i64 final : function
{
  auto tag() const -> function_tag;
};

struct put_u64 final : function
{
  auto tag() const -> function_tag;
};

struct put_f32 final : function
{
  auto tag() const -> function_tag;
};

}  // namespace bython::type_system
#include "builtin.hpp"

namespace
{
auto put_i64_impl(int64_t value) -> void
{
  std::printf("%jd", value);
}

auto putln_i64_impl(int64_t value) -> void
{
  std::printf("%jd\n", value);
}


}  // namespace

namespace bython::codegen
{

auto builtin(builtin_tag tag) -> builtin_metadata
{
  switch (tag) {
    case builtin_tag::put_i64:
      return builtin_metadata {"bython.put_i64", reinterpret_cast<std::uint64_t>(put_i64_impl)};

    case builtin_tag::putln_i64:
      return builtin_metadata {"bython.putln_i64", reinterpret_cast<std::uint64_t>(putln_i64_impl)};
  }
}
}  // namespace bython::codegen
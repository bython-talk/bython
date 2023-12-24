#include <sstream>
#include <string>

#include "type.hpp"

#include "bython/type_system/builtin.hpp"

namespace ts = bython::type_system;
using namespace std::string_literals;

namespace
{
auto type_impl(ts::void_ const& /*void*/) -> std::string
{
  return "void"s;
}

auto type_impl(ts::boolean const& /*boolean*/) -> std::string
{
  return "boolean"s;
}

auto type_impl(ts::uint const& unsigned_int) -> std::string
{
  return "u"s + std::to_string(unsigned_int.width);
}

auto type_impl(ts::sint const& signed_int) -> std::string
{
  return "s"s + std::to_string(signed_int.width);
}

auto type_impl(ts::single_fp const& /*f32*/) -> std::string
{
  return "f32"s;
}

auto type_impl(ts::double_fp const& /*f64*/) -> std::string
{
  return "f64"s;
}

auto type_impl(ts::function_signature const& sig) -> std::string
{
  std::stringstream sstream;
  sstream << Catch::StringMaker<ts::type*>::convert(sig.rettype) << '(';

  for (auto&& parameter : sig.parameters) {
    sstream << Catch::StringMaker<ts::type*>::convert(parameter) << ',';
  }

  return sstream.str();
}
}  // namespace

namespace Catch
{

auto StringMaker<ts::type*>::convert(ts::type* const& value) -> std::string
{
  switch (value->tag()) {
    case ts::type_tag::void_:
      return type_impl(dynamic_cast<ts::void_ const&>(*value));

    case ts::type_tag::boolean:
      return type_impl(dynamic_cast<ts::boolean const&>(*value));

    case ts::type_tag::uint:
      return type_impl(dynamic_cast<ts::uint const&>(*value));

    case ts::type_tag::sint:
      return type_impl(dynamic_cast<ts::sint const&>(*value));

    case ts::type_tag::single_fp:
      return type_impl(dynamic_cast<ts::single_fp const&>(*value));

    case ts::type_tag::double_fp:
      return type_impl(dynamic_cast<ts::double_fp const&>(*value));

    case ts::type_tag::function:
      return type_impl(dynamic_cast<ts::function_signature const&>(*value));
  }
}

}  // namespace Catch
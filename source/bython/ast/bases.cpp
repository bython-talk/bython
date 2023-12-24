#include "bases.hpp"

#include <boost/uuid/uuid.hpp>  // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators

static thread_local auto uuid_generator = boost::uuids::random_generator {};

namespace bython::ast
{
node::node()
    : uuid {uuid_generator()}
{
}
}  // namespace bython::ast
#pragma once

#include "predeclare.h"

#include <memory>
#include <variant>

namespace racc::registry {
    using TypeVariant = std::variant<Struct, Interface, Enum, Alias, FunctionType, TupleType, TypeVar, BuiltinType>;
    using WeakTypeRef = std::weak_ptr<TypeVariant>;
}

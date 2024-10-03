#include "BuiltinType.h"

namespace racc::registry {

    BuiltinType::BuiltinType(std::string name, size_t size)
            : name(std::move(name)),
              size(size) {
    }

    BuiltinType &BuiltinType::operator=(BuiltinType &&) noexcept = default;

    BuiltinType::BuiltinType(BuiltinType &&) noexcept = default;

    BuiltinType::~BuiltinType() = default;

}

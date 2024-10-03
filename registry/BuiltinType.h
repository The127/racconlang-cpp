#pragma once

#include "TypeRef.h"

namespace racc::registry {

    class BuiltinType {
    public:
        std::string name;
        size_t size;
        WeakTypeRef type;

        BuiltinType(std::string name, size_t size);

        ~BuiltinType();

        BuiltinType(const BuiltinType &) = delete;

        BuiltinType &operator=(const BuiltinType &) = delete;

        BuiltinType(BuiltinType &&) noexcept;

        BuiltinType &operator=(BuiltinType &&) noexcept;

    };

}

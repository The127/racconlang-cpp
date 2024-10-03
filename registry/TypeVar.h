#pragma once

#include "TypeRef.h"

#include <string>
#include <utility>

namespace racc::registry {

    class TypeVar {
    public:
        std::string name;
        WeakTypeRef type;

        explicit TypeVar(std::string name);

        ~TypeVar();

        TypeVar(const TypeVar &) = delete;

        TypeVar &operator=(const TypeVar &) = delete;

        TypeVar(TypeVar &&) noexcept;

        TypeVar &operator=(TypeVar &&) noexcept;

    };

}

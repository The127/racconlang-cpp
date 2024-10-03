#pragma once

#include "TypeRef.h"

#include <vector>
#include <memory>

namespace racc::registry {

    class TupleType {
    public:
        std::vector<TypeRef> types;
        WeakTypeRef type;

        TupleType(std::vector<TypeRef> types);

        ~TupleType();

        TupleType(const TupleType &) = delete;

        TupleType &operator=(const TupleType &) = delete;

        TupleType(TupleType &&) noexcept;

        TupleType &operator=(TupleType &&) noexcept;

    };

}

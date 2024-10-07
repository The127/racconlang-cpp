#pragma once

#include "Id.h"
#include "TypeRef.h"
#include "TypeBase.h"

class racc::registry::BuiltinType : public TypeBase<BuiltinType> {
public:
    Id name;
    size_t size;

    BuiltinType(Id name, size_t size);

    ~BuiltinType();

    BuiltinType(const BuiltinType &) = delete;

    BuiltinType &operator=(const BuiltinType &) = delete;

    BuiltinType(BuiltinType &&) noexcept;

    BuiltinType &operator=(BuiltinType &&) noexcept;

};

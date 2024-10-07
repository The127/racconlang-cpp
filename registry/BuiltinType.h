#pragma once

#include "TypeRef.h"
#include "TypeBase.h"

class racc::registry::BuiltinType : public TypeBase<BuiltinType> {
public:
    std::string name;
    size_t size;

    BuiltinType(std::string name, size_t size);

    ~BuiltinType();

    BuiltinType(const BuiltinType &) = delete;

    BuiltinType &operator=(const BuiltinType &) = delete;

    BuiltinType(BuiltinType &&) noexcept;

    BuiltinType &operator=(BuiltinType &&) noexcept;

};

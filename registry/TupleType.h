#pragma once

#include "TypeRef.h"
#include "TypeBase.h"

#include <vector>
#include <memory>

class racc::registry::TupleType : public TypeBase<TupleType> {
public:
    std::vector<TypeRef> types;

    explicit TupleType(std::vector<TypeRef> types);

    ~TupleType();

    TupleType(const TupleType &) = delete;

    TupleType &operator=(const TupleType &) = delete;

    TupleType(TupleType &&) noexcept;

    TupleType &operator=(TupleType &&) noexcept;

};

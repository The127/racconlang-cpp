#pragma once

#include "Id.h"
#include "TypeRef.h"
#include "TypeBase.h"

#include <string>
#include <utility>

class racc::registry::TypeVar : public TypeBase<TypeVar> {
public:
    Id name;

    explicit TypeVar(Id name);

    ~TypeVar();

    TypeVar(const TypeVar &) = delete;

    TypeVar &operator=(const TypeVar &) = delete;

    TypeVar(TypeVar &&) noexcept;

    TypeVar &operator=(TypeVar &&) noexcept;

};

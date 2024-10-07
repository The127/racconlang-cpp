#pragma once

#include "TypeRef.h"
#include "TypeBase.h"

#include <string>
#include <utility>

class racc::registry::TypeVar : public TypeBase<TypeVar> {
public:
    std::string name;

    explicit TypeVar(std::string name);

    ~TypeVar();

    TypeVar(const TypeVar &) = delete;

    TypeVar &operator=(const TypeVar &) = delete;

    TypeVar(TypeVar &&) noexcept;

    TypeVar &operator=(TypeVar &&) noexcept;

};

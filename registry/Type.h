//
// Created by zoe on 23.09.24.
//


#pragma once
#include <variant>

#include "Enum.h"
#include "Interface.h"
#include "Struct.h"


class Type {
public:
    std::variant<Struct, Interface, Enum> type;

    explicit(false) Type(Struct s);
    explicit(false) Type(Interface i);
    explicit(false) Type(Enum e);

    [[nodiscard]] bool isStruct() const;
    [[nodiscard]] bool isInterface() const;
    [[nodiscard]] bool isEnum() const;
};

//
// Created by zoe on 23.09.24.
//


#pragma once
#include <variant>

#include "Enum.h"
#include "Interface.h"
#include "Struct.h"
#include "Alias.h"


class Type {
public:
    std::variant<Struct, Interface, Enum, Alias> type;

    explicit(false) Type(Struct s);
    explicit(false) Type(Interface i);
    explicit(false) Type(Enum e);
    explicit(false) Type(Alias a);

    [[nodiscard]] bool isStruct() const;
    [[nodiscard]] bool isInterface() const;
    [[nodiscard]] bool isEnum() const;
    [[nodiscard]] bool isAlias() const;

    void populate();
};

//
// Created by zoe on 23.09.24.
//

#include "Type.h"

Type::Type(Struct s)
    : type(std::move(s)) {
}

Type::Type(Interface i)
    : type(std::move(i)){
}

Type::Type(Enum e)
    : type(std::move(e)){
}

bool Type::isStruct() const {
    return std::holds_alternative<Struct>(type);
}

bool Type::isInterface() const {
    return std::holds_alternative<Interface>(type);
}

bool Type::isEnum() const {
    return std::holds_alternative<Enum>(type);
}

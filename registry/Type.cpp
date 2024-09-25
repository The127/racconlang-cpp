//
// Created by zoe on 23.09.24.
//

#include "Type.h"


Type::Type(Struct s) : type(std::move(s)) {}

Type::Type(Interface i) : type(std::move(i)) {}

Type::Type(Enum e) : type(std::move(e)) {}

Type::Type(Alias a) : type(std::move(a)) {}

bool Type::isStruct() const {
    return std::holds_alternative<Struct>(type);
}

bool Type::isInterface() const {
    return std::holds_alternative<Interface>(type);
}

bool Type::isEnum() const {
    return std::holds_alternative<Enum>(type);
}

bool Type::isAlias() const {
    return std::holds_alternative<Alias>(type);
}

void Type::populate() {
    if (isStruct()) {
        std::get<Struct>(type).populate();
    }
    if (isInterface()) {
        std::get<Interface>(type).populate();
    }
    if (isEnum()) {
        std::get<Enum>(type).populate();
    }
    if (isAlias()) {
        std::get<Alias>(type).populate();
    }
}

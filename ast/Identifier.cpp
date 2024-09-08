//
// Created by zoe on 07.09.24.
//

#include "Identifier.h"
#include "sourceMap/Source.h"

Identifier::Identifier(const Token &identifier, const Source &source): identifier(identifier) {
    name = source.getText(identifier.start, identifier.end);
    if (name.starts_with('@'))
        name = name.substr(1, name.size());
}

uint64_t Identifier::start() const {
    return identifier.start;
}

uint64_t Identifier::end() const {
    return identifier.end;
}

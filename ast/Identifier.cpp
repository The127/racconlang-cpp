//
// Created by zoe on 07.09.24.
//

#include "Identifier.h"
#include "sourceMap/Source.h"

Identifier::Identifier(const Token &identifier, const Source &source): identifier(identifier) {
    name = source.getText(identifier.start, identifier.end);
    if (name.starts_with('@'))
        name = name.substr(1);
}

uint64_t Identifier::start() const {
    return identifier.start;
}

uint64_t Identifier::end() const {
    return identifier.end;
}

std::string Identifier::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    return std::move(std::string(name));
}

std::ostream & operator<<(std::ostream &out, const Identifier &identifier) {
    out << identifier.name;
    return out;
}

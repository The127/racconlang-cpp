//
// Created by zoe on 07.09.24.
//

#include "Identifier.h"
#include "sourceMap/Source.h"

Identifier::~Identifier() = default;
Identifier::Identifier(const Identifier &) = default;
Identifier & Identifier::operator=(const Identifier &) = default;
Identifier::Identifier(Identifier &&) noexcept = default;
Identifier & Identifier::operator=(Identifier &&) noexcept = default;

Identifier Identifier::make(const Token &identifier, const std::shared_ptr<Source> &source) {
    auto name = source->getText(identifier.start, identifier.end);
    if (name.starts_with('@'))
        name = name.substr(1);
    return Identifier(identifier, name);
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

Identifier::Identifier(const Token &identifier, const std::string_view &name): identifier(identifier), name(name) {}

std::ostream & operator<<(std::ostream &out, const Identifier &identifier) {
    out << identifier.name;
    return out;
}

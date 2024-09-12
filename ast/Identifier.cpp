//
// Created by zoe on 07.09.24.
//

#include "Identifier.h"
#include "sourceMap/Source.h"

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

//
// Created by zoe on 07.09.24.
//

#include "PropertyDeclaration.h"

#include "utils/NodeUtils.h"

PropertyDeclaration::PropertyDeclaration(const Identifier &name): name(name) {}
PropertyDeclaration::PropertyDeclaration(PropertyDeclaration &&) noexcept = default;
PropertyDeclaration & PropertyDeclaration::operator=(PropertyDeclaration &&) noexcept = default;

PropertyDeclaration::~PropertyDeclaration() = default;

uint64_t PropertyDeclaration::start() const {
    return startPos;
}

uint64_t PropertyDeclaration::end() const {
    return endPos;
}

std::string PropertyDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "PropertyDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";
    result += std::string(indent, ' ') + "name: " + name.toString(sources, indent+1, verbose) + ",\n";
    if(type) {
        result += std::string(indent, ' ') + "type: " + (*type)->toString(sources, indent+1, verbose) + ",\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return result;
}

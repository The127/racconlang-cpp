//
// Created by zoe on 12.09.24.
//

#include "InterfaceSetterDeclaration.h"

#include "utils/NodeUtils.h"

InterfaceSetterDeclaration::InterfaceSetterDeclaration() = default;
InterfaceSetterDeclaration::InterfaceSetterDeclaration(InterfaceSetterDeclaration &&) noexcept = default;
InterfaceSetterDeclaration & InterfaceSetterDeclaration::operator=(InterfaceSetterDeclaration &&) noexcept = default;
InterfaceSetterDeclaration::~InterfaceSetterDeclaration() = default;

uint64_t InterfaceSetterDeclaration::start() const {
    return startPos;
}

uint64_t InterfaceSetterDeclaration::end() const {
    return endPos;
}

std::string InterfaceSetterDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "InterfaceSetter", verbose) + "{\n";

    if(name) {
        result += std::string(indent, ' ') + "name: " + name->toString(sources, indent+1, verbose) + ",\n";
    }

    if(parameter) {
        result += std::string(indent, ' ') + "parameter: " + parameter->toString(sources, indent+1, verbose) + ",\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return result;
}

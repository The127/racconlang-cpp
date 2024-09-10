//
// Created by zoe on 07.09.24.
//

#include "InterfaceConstraint.h"

#include "utils/NodeUtils.h"

uint64_t InterfaceConstraint::start() const {
    return typeSignature.start();
}

uint64_t InterfaceConstraint::end() const {
    return typeSignature.end();
}

std::string InterfaceConstraint::toString(const SourceMap &sources, int indent, bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "InterfaceConstraint", verbose) + "{\n";

    result += std::string(indent, ' ') + "typeSignature: " + typeSignature.toString(sources, indent+1, verbose) + ",\n";

    result += std::string(indent - 1, ' ') + "},";
    return std::move(result);
}

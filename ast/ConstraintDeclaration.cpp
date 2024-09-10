//
// Created by zoe on 07.09.24.
//

#include "ConstraintDeclaration.h"

#include "utils/NodeUtils.h"

uint64_t ConstraintDeclaration::start() const {
    return startPos;
}

uint64_t ConstraintDeclaration::end() const {
    return endPos;
}

std::string ConstraintDeclaration::toString(const SourceMap &sources, int indent, bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ConstraintDeclaration", verbose) + "{\n";

    if(name) {
        result += std::string(indent, ' ') + "name: " + name->toString(sources, indent+1, verbose) + ",\n";
    }

    if(!constraints.empty()) {
        result += std::string(indent, ' ') + "constraints: " + NodeUtils::nodeListString(sources, constraints, indent + 1, verbose) + "\n";
    }

    result += std::string(indent - 1, ' ') + "},";
    return std::move(result);
}

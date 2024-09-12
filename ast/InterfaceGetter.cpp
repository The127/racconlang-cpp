//
// Created by zoe on 12.09.24.
//

#include "InterfaceGetter.h"

#include "utils/NodeUtils.h"

uint64_t InterfaceGetter::start() const {
    return startPos;
}

uint64_t InterfaceGetter::end() const {
    return endPos;
}

std::string InterfaceGetter::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "InterfaceGetter", verbose) + "{\n";

    result += std::string(indent, ' ') + "isMut: " + std::to_string(isMut) + ",\n";

    if (name) {
        result += std::string(indent, ' ') + "name: " + name->toString(sources, indent+1, verbose) + ",\n";
    }

    if (returnType) {
        result += std::string(indent, ' ') + "returnType: " + returnType->toString(sources, indent+1, verbose) + ",\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}

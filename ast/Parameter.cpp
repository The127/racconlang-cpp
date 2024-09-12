//
// Created by zoe on 10.09.24.
//

#include "Parameter.h"

#include "utils/NodeUtils.h"

uint64_t Parameter::start() const {
    return startPos;
}

uint64_t Parameter::end() const {
    return endPos;
}

std::string Parameter::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "Parameter", verbose) + "{\n";

    result += std::string(indent, ' ') + "isMut: " + std::to_string(isMut) + ",\n";
    result += std::string(indent, ' ') + "isRef: " + std::to_string(isRef) + ",\n";
    result += std::string(indent, ' ') + "name: " + name.toString(sources, indent+1, verbose) + ",\n";
    result += std::string(indent, ' ') + "type: " + type->toString(sources, indent+1, verbose) + ",\n";

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}

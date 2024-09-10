//
// Created by zoe on 07.09.24.
//

#include "EnumMemberDeclaration.h"

#include "utils/NodeUtils.h"

uint64_t EnumMemberDeclaration::start() const {
    return startPos;
}

uint64_t EnumMemberDeclaration::end() const {
    return endPos;
}

std::string EnumMemberDeclaration::toString(const SourceMap &sources, int indent, bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "EnumMemberDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "name: " + name.toString(sources, indent+1, verbose) + ",\n";

    if(!values.empty()) {
        result += std::string(indent, ' ') + "values: " + NodeUtils::nodeListString(sources, genericArguments, indent + 1, verbose) + "\n";
    }

    return std::move(result);
}

//
// Created by zoe on 07.09.24.
//

#include "EnumDeclaration.h"

#include "utils/NodeUtils.h"

uint64_t EnumDeclaration::start() const {
    return startPos;
}

uint64_t EnumDeclaration::end() const {
    return endPos;
}

std::string EnumDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "EnumDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

    result += std::string(indent, ' ') + "name: ";
    if(name) {
        result += name->name;
    }
    result += ",\n";

    if(!genericParams.empty())
        result += std::string(indent, ' ') + "genericParams: " + NodeUtils::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";

    if(!genericConstraints.empty())
        result += std::string(indent, ' ') + "genericConstraints: " + NodeUtils::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";

    if(!memberDeclarations.empty())
        result += std::string(indent, ' ') + "memberDeclarations: " + NodeUtils::nodeListString(sources, memberDeclarations, indent + 1, verbose) + "\n";

    result += std::string(indent, ' ') + "},";
    return std::move(result);
}

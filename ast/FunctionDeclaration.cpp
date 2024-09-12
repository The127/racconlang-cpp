//
// Created by zoe on 07.09.24.
//

#include "FunctionDeclaration.h"

#include "utils/NodeUtils.h"

uint64_t FunctionDeclaration::start() const {
    return startPos;
}

uint64_t FunctionDeclaration::end() const {
    return endPos;
}

std::string FunctionDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "FunctionDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

    if(name) {
        result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
    }

    if(!genericParams.empty())
        result += std::string(indent, ' ') + "genericParams: " + NodeUtils::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";

    if(!genericConstraints.empty())
        result += std::string(indent, ' ') + "genericConstraints: " + NodeUtils::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";

    if(!parameters.empty())
        result += std::string(indent, ' ') + "parameters: " + NodeUtils::nodeListString(sources, parameters, indent + 1, verbose) + "\n";

    result += std::string(indent, ' ') + "returnType: " + returnType->toString(sources, indent + 1, verbose) + "\n";

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}

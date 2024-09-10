//
// Created by zoe on 07.09.24.
//

#include "InterfaceMethodDeclaration.h"

#include "utils/NodeUtils.h"

uint64_t InterfaceMethodDeclaration::start() const {
    return startPos;
}

uint64_t InterfaceMethodDeclaration::end() const {
    return endPos;
}

std::string InterfaceMethodDeclaration::toString(const SourceMap &sources, int indent, bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "InterfaceMethodDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "name: " + name.toString(sources, indent+1, verbose) + ",\n";

    if(!genericParams.empty()) {
        result += std::string(indent, ' ') + "genericParams: " + NodeUtils::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";
    }
    if(!genericConstraints.empty()) {
        result += std::string(indent, ' ') + "genericConstraints: " + NodeUtils::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";
    }
    if(!parameters.empty()) {
        result += std::string(indent, ' ') + "parameterTypes: " + NodeUtils::nodeListString(sources, parameters, indent + 1, verbose) + "\n";
    }

    if(returnType) {
        result += std::string(indent, ' ') + "returnType: " + (*returnType)->toString(sources, indent+1, verbose) + ",\n";
    }

    return std::move(result);
}

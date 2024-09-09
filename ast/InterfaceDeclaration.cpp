//
// Created by zoe on 07.09.24.
//

#include "InterfaceDeclaration.h"

#include "utils/NodeUtils.h"

uint64_t InterfaceDeclaration::start() const {
    return startPos;
}

uint64_t InterfaceDeclaration::end() const {
    return endPos;
}

std::string InterfaceDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "InterfaceDeclaration", verbose) + "{\n";

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

    if(!requiredInterfaces.empty())
        result += std::string(indent, ' ') + "requiredInterfaces: " + NodeUtils::nodeListString(sources, requiredInterfaces, indent + 1, verbose) + "\n";

    if(!methods.empty())
        result += std::string(indent, ' ') + "methods: " + NodeUtils::nodeListString(sources, methods, indent + 1, verbose) + "\n";

    result += std::string(indent, ' ') + "},";
    return std::move(result);
}

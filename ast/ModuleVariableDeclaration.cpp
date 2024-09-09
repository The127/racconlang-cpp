//
// Created by zoe on 07.09.24.
//

#include "ModuleVariableDeclaration.h"

#include "utils/NodeUtils.h"

uint64_t ModuleVariableDeclaration::start() const {
    return startPos;
}

uint64_t ModuleVariableDeclaration::end() const {
    return endPos;
}

std::string ModuleVariableDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ModuleVariableDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

    result += std::string(indent, ' ') + "name: ";
    if(name) {
        result += name->name;
    }
    result += ",\n";

    result += std::string(indent, ' ') + "returnType: " + type->toString(sources, indent + 1, verbose) + "\n";

    result += std::string(indent, ' ') + "},";
    return std::move(result);
}

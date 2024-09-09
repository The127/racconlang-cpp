//
// Created by zoe on 07.09.24.
//

#include "ModuleDeclaration.h"
#include "utils/NodeUtils.h"

uint64_t ModuleDeclaration::start() const {
    return startPos;
}

uint64_t ModuleDeclaration::end() const {
    return endPos;
}

std::string ModuleDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ModuleDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "path: " + path.toString(sources, indent, verbose) + ",\n";

    if(!uses.empty())
        result += std::string(indent, ' ') + "uses: " + NodeUtils::nodeListString(sources, uses, indent + 1, verbose) + "\n";

    if(!moduleVariableDeclarations.empty())
        result += std::string(indent, ' ') + "moduleVariableDeclarations: " + NodeUtils::nodeListString(sources, moduleVariableDeclarations, indent + 1, verbose) + "\n";

    if(!aliasDeclarations.empty())
        result += std::string(indent, ' ') + "aliasDeclarations: " + NodeUtils::nodeListString(sources, aliasDeclarations, indent + 1, verbose) + "\n";

    if(!enumDeclarations.empty())
        result += std::string(indent, ' ') + "enumDeclarations: " + NodeUtils::nodeListString(sources, enumDeclarations, indent + 1, verbose) + "\n";

    if(!interfaceDeclarations.empty())
        result += std::string(indent, ' ') + "interfaceDeclarations: " + NodeUtils::nodeListString(sources, interfaceDeclarations, indent + 1, verbose) + "\n";

    if(!structDeclarations.empty())
        result += std::string(indent, ' ') + "structDeclarations: " + NodeUtils::nodeListString(sources, structDeclarations, indent + 1, verbose) + "\n";

    if(!functionDeclarations.empty())
        result += std::string(indent, ' ') + "functionDeclarations: " + NodeUtils::nodeListString(sources, functionDeclarations, indent + 1, verbose) + "\n";

    result += std::string(indent, ' ') + "},";
    return std::move(result);
}

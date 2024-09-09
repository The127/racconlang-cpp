//
// Created by zoe on 07.09.24.
//

#include "StructDeclaration.h"

#include "utils/NodeUtils.h"
#include "utils/StringUtils.h"

uint64_t StructDeclaration::start() const {
    return startPos;
}

uint64_t StructDeclaration::end() const {
    return endPos;
}

std::string StructDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "StructDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

    result += std::string(indent, ' ') + "name: ";
    if(name) {
        result += name->name;
    }
    result += ",\n";

    result += std::string(indent, ' ') + "genericParams: " + NodeUtils::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";
    result += std::string(indent, ' ') + "genericConstraints: " + NodeUtils::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";
    result += std::string(indent, ' ') + "propertyDeclarations: " + NodeUtils::nodeListString(sources, propertyDeclarations, indent + 1, verbose) + "\n";

    result += std::string(indent, ' ') + "destructuresInto: [" + StringUtils::join(destructureProperties, ", ") + "],";

    result += std::string(indent, ' ') + "},";
    return std::move(result);
}

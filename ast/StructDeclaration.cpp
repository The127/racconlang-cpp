//
// Created by zoe on 07.09.24.
//

#include "StructDeclaration.h"
#include "PropertyDeclaration.h"
#include "ConstraintDeclaration.h"

#include "utils/NodeUtils.h"
#include "utils/StringUtils.h"

StructDeclaration::StructDeclaration() = default;
StructDeclaration::StructDeclaration(StructDeclaration &&) noexcept = default;
StructDeclaration & StructDeclaration::operator=(StructDeclaration &&) noexcept = default;
StructDeclaration::~StructDeclaration() = default;

uint64_t StructDeclaration::start() const {
    return startPos;
}

uint64_t StructDeclaration::end() const {
    return endPos;
}

std::string StructDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "StructDeclaration", verbose) + "{\n";

    result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

    if(name) {
        result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
    }

    if(!genericParams.empty())
        result += std::string(indent, ' ') + "genericParams: " + NodeUtils::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";

    if(!genericConstraints.empty())
        result += std::string(indent, ' ') + "genericConstraints: " + NodeUtils::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";

    if(!propertyDeclarations.empty())
        result += std::string(indent, ' ') + "propertyDeclarations: " + NodeUtils::nodeListString(sources, propertyDeclarations, indent + 1, verbose) + "\n";

    result += std::string(indent, ' ') + "destructuresInto: [" + StringUtils::join(destructureProperties, ", ") + "],";

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}

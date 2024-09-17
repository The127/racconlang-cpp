//
// Created by zoe on 14.09.24.
//

#include "ConstructorDeclaration.h"
#include "Parameter.h"

#include "utils/NodeUtils.h"

ConstructorDeclaration::ConstructorDeclaration() = default;
ConstructorDeclaration::ConstructorDeclaration(ConstructorDeclaration &&) noexcept = default;
ConstructorDeclaration & ConstructorDeclaration::operator=(ConstructorDeclaration &&) noexcept = default;
ConstructorDeclaration::~ConstructorDeclaration() = default;

uint64_t ConstructorDeclaration::start() const {
    return startPos;
}

uint64_t ConstructorDeclaration::end() const {
    return endPos;
}

std::string ConstructorDeclaration::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ConstructorDeclaration", verbose) + "{\n";

    if(!parameters.empty()) {
        result += std::string(indent, ' ') + "parameters: " + NodeUtils::nodeListString(sources, parameters, indent + 1, verbose) + "\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}

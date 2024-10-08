//
// Created by zoe on 07.09.24.
//

#include "EnumDeclaration.h"
#include "EnumMemberDeclaration.h"
#include "ConstraintDeclaration.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    EnumDeclaration::EnumDeclaration() = default;

    EnumDeclaration::EnumDeclaration(EnumDeclaration &&) noexcept = default;

    EnumDeclaration &EnumDeclaration::operator=(EnumDeclaration &&) noexcept = default;

    EnumDeclaration::~EnumDeclaration() = default;

    uint64_t EnumDeclaration::start() const {
        return startPos;
    }

    uint64_t EnumDeclaration::end() const {
        return endPos;
    }

    std::string EnumDeclaration::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "EnumDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
        };

        if (!genericParams.empty())
            result += std::string(indent, ' ') + "genericParams: " + utils::node::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";

        if (!genericConstraints.empty())
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";

        if (!memberDeclarations.empty())
            result += std::string(indent, ' ') + "memberDeclarations: " + utils::node::nodeListString(sources, memberDeclarations, indent + 1, verbose) + "\n";

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
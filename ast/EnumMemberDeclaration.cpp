//
// Created by zoe on 07.09.24.
//

#include "EnumMemberDeclaration.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    EnumMemberDeclaration::EnumMemberDeclaration(Identifier name)
            : name(std::move(name)) {
    }

    EnumMemberDeclaration::EnumMemberDeclaration(EnumMemberDeclaration &&) noexcept = default;

    EnumMemberDeclaration &EnumMemberDeclaration::operator=(EnumMemberDeclaration &&) noexcept = default;

    EnumMemberDeclaration::~EnumMemberDeclaration() = default;

    uint64_t EnumMemberDeclaration::start() const {
        return startPos;
    }

    uint64_t EnumMemberDeclaration::end() const {
        return endPos;
    }

    std::string EnumMemberDeclaration::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "EnumMemberDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "name: " + name.toString(sources, indent + 1, verbose) + ",\n";

        if (!values.empty()) {
            result += std::string(indent, ' ') + "values: " + utils::node::nodeListString(sources, values, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
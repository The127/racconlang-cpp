//
// Created by zoe on 07.09.24.
//

#include "ConstraintDeclaration.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    ConstraintDeclaration::ConstraintDeclaration() = default;

    ConstraintDeclaration::ConstraintDeclaration(ConstraintDeclaration &&) noexcept = default;

    ConstraintDeclaration &ConstraintDeclaration::operator=(ConstraintDeclaration &&) noexcept = default;

    ConstraintDeclaration::~ConstraintDeclaration() = default;

    uint64_t ConstraintDeclaration::start() const {
        return startPos;
    }

    uint64_t ConstraintDeclaration::end() const {
        return endPos;
    }

    std::string ConstraintDeclaration::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "ConstraintDeclaration", verbose) + "{\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + name->toString(sources, indent + 1, verbose) + ",\n";
        }

        if (!constraints.empty()) {
            result += std::string(indent, ' ') + "constraints: " + utils::node::nodeListString(sources, constraints, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
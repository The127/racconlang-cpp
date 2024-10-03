//
// Created by zoe on 14.09.24.
//

#include "DestructureDeclaration.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    DestructureDeclaration::DestructureDeclaration() = default;

    DestructureDeclaration::DestructureDeclaration(DestructureDeclaration &&) noexcept = default;

    DestructureDeclaration &DestructureDeclaration::operator=(DestructureDeclaration &&) noexcept = default;

    DestructureDeclaration::~DestructureDeclaration() = default;

    uint64_t DestructureDeclaration::start() const {
        return startPos;
    }

    uint64_t DestructureDeclaration::end() const {
        return endPos;
    }

    std::string DestructureDeclaration::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "DestructureDeclaration", verbose) + "{\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
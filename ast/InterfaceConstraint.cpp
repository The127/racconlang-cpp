//
// Created by zoe on 07.09.24.
//

#include "InterfaceConstraint.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    uint64_t InterfaceConstraint::start() const {
        return typeSignature.start();
    }

    uint64_t InterfaceConstraint::end() const {
        return typeSignature.end();
    }

    InterfaceConstraint::InterfaceConstraint() = default;

    InterfaceConstraint::InterfaceConstraint(InterfaceConstraint &&other) noexcept = default;

    InterfaceConstraint &InterfaceConstraint::operator=(InterfaceConstraint &&other) noexcept = default;

    InterfaceConstraint::~InterfaceConstraint() = default;

    std::string InterfaceConstraint::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "InterfaceConstraint", verbose) + "{\n";

        result += std::string(indent, ' ') + "typeSignature: " + typeSignature.toString(sources, indent + 1, verbose) + ",\n";

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
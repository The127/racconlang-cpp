//
// Created by zoe on 07.09.24.
//

#include "InterfaceMethodDeclaration.h"

#include "Parameter.h"
#include "ConstraintDeclaration.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    InterfaceMethodDeclaration::InterfaceMethodDeclaration() = default;

    InterfaceMethodDeclaration::InterfaceMethodDeclaration(InterfaceMethodDeclaration &&) noexcept = default;

    InterfaceMethodDeclaration &InterfaceMethodDeclaration::operator=(InterfaceMethodDeclaration &&other) noexcept = default;

    InterfaceMethodDeclaration::~InterfaceMethodDeclaration() = default;

    uint64_t InterfaceMethodDeclaration::start() const {
        return startPos;
    }

    uint64_t InterfaceMethodDeclaration::end() const {
        return endPos;
    }

    std::string InterfaceMethodDeclaration::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "InterfaceMethodDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "isMut: " + std::to_string(isMut) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + name->toString(sources, indent + 1, verbose) + ",\n";
        }

        if (!genericParams.empty()) {
            result += std::string(indent, ' ') + "genericParams: " + utils::node::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";
        }
        if (!genericConstraints.empty()) {
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";
        }
        if (!parameters.empty()) {
            result += std::string(indent, ' ') + "parameters: " + utils::node::nodeListString(sources, parameters, indent + 1, verbose) + "\n";
        }

        if (returnType) {
            result += std::string(indent, ' ') + "returnType: " + returnType->toString(sources, indent + 1, verbose) + ",\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }
}


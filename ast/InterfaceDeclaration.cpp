//
// Created by zoe on 07.09.24.
//

#include "InterfaceDeclaration.h"
#include "ConstraintDeclaration.h"
#include "InterfaceGetterDeclaration.h"
#include "InterfaceMethodDeclaration.h"
#include "InterfaceSetterDeclaration.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    InterfaceDeclaration::InterfaceDeclaration() = default;

    InterfaceDeclaration::InterfaceDeclaration(InterfaceDeclaration &&) noexcept = default;

    InterfaceDeclaration &InterfaceDeclaration::operator=(InterfaceDeclaration &&) noexcept = default;

    InterfaceDeclaration::~InterfaceDeclaration() = default;

    uint64_t InterfaceDeclaration::start() const {
        return startPos;
    }

    uint64_t InterfaceDeclaration::end() const {
        return endPos;
    }

    std::string InterfaceDeclaration::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "InterfaceDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
        }

        if (!genericParams.empty())
            result += std::string(indent, ' ') + "genericParams: " + utils::node::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";

        if (!genericConstraints.empty())
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";

        if (!requiredInterfaces.empty())
            result += std::string(indent, ' ') + "requiredInterfaces: " + utils::node::nodeListString(sources, requiredInterfaces, indent + 1, verbose) + "\n";

        if (!methods.empty())
            result += std::string(indent, ' ') + "methods: " + utils::node::nodeListString(sources, methods, indent + 1, verbose) + "\n";
        if (!getters.empty())
            result += std::string(indent, ' ') + "getters: " + utils::node::nodeListString(sources, getters, indent + 1, verbose) + "\n";
        if (!setters.empty())
            result += std::string(indent, ' ') + "setters: " + utils::node::nodeListString(sources, setters, indent + 1, verbose) + "\n";

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}

//
// Created by zoe on 12.09.24.
//

#include "InterfaceGetterDeclaration.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    InterfaceGetterDeclaration::InterfaceGetterDeclaration() = default;

    InterfaceGetterDeclaration::InterfaceGetterDeclaration(InterfaceGetterDeclaration &&) noexcept = default;

    InterfaceGetterDeclaration &InterfaceGetterDeclaration::operator=(InterfaceGetterDeclaration &&) noexcept = default;

    InterfaceGetterDeclaration::~InterfaceGetterDeclaration() = default;

    uint64_t InterfaceGetterDeclaration::start() const {
        return startPos;
    }

    uint64_t InterfaceGetterDeclaration::end() const {
        return endPos;
    }

    std::string InterfaceGetterDeclaration::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "InterfaceGetter", verbose) + "{\n";

        result += std::string(indent, ' ') + "isMut: " + std::to_string(isMut) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + name->toString(sources, indent + 1, verbose) + ",\n";
        }

        if (returnType) {
            result += std::string(indent, ' ') + "returnType: " + returnType->toString(sources, indent + 1, verbose) + ",\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
//
// Created by zoe on 14.09.24.
//

#include "ConstructorDeclaration.h"
#include "Parameter.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    ConstructorDeclaration::ConstructorDeclaration() = default;

    ConstructorDeclaration::ConstructorDeclaration(ConstructorDeclaration &&) noexcept = default;

    ConstructorDeclaration &ConstructorDeclaration::operator=(ConstructorDeclaration &&) noexcept = default;

    ConstructorDeclaration::~ConstructorDeclaration() = default;

    uint64_t ConstructorDeclaration::start() const {
        return startPos;
    }

    uint64_t ConstructorDeclaration::end() const {
        return endPos;
    }

    std::string ConstructorDeclaration::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "ConstructorDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
        }

        if (!parameters.empty()) {
            result += std::string(indent, ' ') + "parameters: " + utils::node::nodeListString(sources, parameters, indent + 1, verbose) + "\n";
        }

        if (otherName) {
            result += std::string(indent, ' ') + "otherName: " + std::string(otherName->name) + ",\n";
        }

        //TODO: expressions for other name

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}

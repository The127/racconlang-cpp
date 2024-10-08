//
// Created by zoe on 14.09.24.
//

#include "ImplMethod.h"

#include "utils/NodeUtils.h"
#include "Parameter.h"
#include "ConstraintDeclaration.h"

namespace racc::ast {

    ImplMethod::ImplMethod() = default;

    ImplMethod::ImplMethod(ImplMethod &&) noexcept = default;

    ImplMethod &ImplMethod::operator=(ImplMethod &&) noexcept = default;

    ImplMethod::~ImplMethod() = default;

    uint64_t ImplMethod::start() const {
        return startPos;
    }

    uint64_t ImplMethod::end() const {
        return endPos;
    }

    std::string ImplMethod::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "ImplMethod", verbose) + "{\n";

        result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";
        result += std::string(indent, ' ') + "isStatic: " + std::to_string(isPublic) + ",\n";
        result += std::string(indent, ' ') + "isMut: " + std::to_string(isMut) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + name->toString(sources, indent + 1, verbose) + "\n";
        }

        if (!genericParams.empty()) {
            result += std::string(indent, ' ') + "genericParams: " + utils::node::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";
        }

        if (!genericConstraints.empty()) {
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";
        }

        if (!genericConstraints.empty()) {
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";
        }

        if (returnType) {
            result += std::string(indent, ' ') + returnType->toString(sources, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}

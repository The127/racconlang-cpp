//
// Created by zoe on 14.09.24.
//

#include "ImplGetter.h"
#include "ReturnType.h"

#include "utils/NodeUtils.h"

namespace racc::ast {

    ImplGetter::ImplGetter() = default;

    ImplGetter::ImplGetter(ImplGetter &&) noexcept = default;

    ImplGetter &ImplGetter::operator=(ImplGetter &&) noexcept = default;

    ImplGetter::~ImplGetter() = default;

    uint64_t ImplGetter::start() const {
        return startPos;
    }

    uint64_t ImplGetter::end() const {
        return endPos;
    }

    std::string ImplGetter::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "ImplGetter", verbose) + "{\n";

        result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";
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

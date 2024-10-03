//
// Created by zoe on 07.09.24.
//

#include "utils/NodeUtils.h"
#include "Signature.h"

#include "TypeSignature.h"

namespace racc::ast {

    TypeSignature::TypeSignature() = default;

    TypeSignature::TypeSignature(TypeSignature &&) noexcept = default;

    TypeSignature &TypeSignature::operator=(TypeSignature &&) noexcept = default;

    TypeSignature::~TypeSignature() = default;

    uint64_t TypeSignature::start() const {
        return startPos;
    }

    uint64_t TypeSignature::end() const {
        return endPos;
    }

    std::string TypeSignature::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "TypeSignature", verbose) + "{\n";

        result += std::string(indent, ' ') + "path: " + path.toString(sources, indent + 1, verbose) + ",\n";

        if (!genericArguments.empty()) {
            result += std::string(indent, ' ') + "genericArguments: " + utils::node::nodeListString(sources, genericArguments, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
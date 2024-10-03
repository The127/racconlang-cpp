//
// Created by zoe on 10.09.24.
//

#include "utils/NodeUtils.h"
#include "Signature.h"

#include "TupleSignature.h"

namespace racc::ast {

    TupleSignature::TupleSignature() = default;

    TupleSignature::TupleSignature(TupleSignature &&) noexcept = default;

    TupleSignature &TupleSignature::operator=(TupleSignature &&) noexcept = default;

    TupleSignature::~TupleSignature() = default;

    uint64_t TupleSignature::start() const {
        return startPos;
    }

    uint64_t TupleSignature::end() const {
        return endPos;
    }

    std::string TupleSignature::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "TupleSignature", verbose) + "{\n";

        if (!types.empty()) {
            result += std::string(indent, ' ') + "types: " + utils::node::nodeListString(sources, types, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
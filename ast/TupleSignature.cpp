//
// Created by zoe on 10.09.24.
//

#include "TupleSignature.h"

#include "utils/NodeUtils.h"

TupleSignature::TupleSignature() = default;
TupleSignature::TupleSignature(TupleSignature &&) noexcept = default;
TupleSignature & TupleSignature::operator=(TupleSignature &&) noexcept = default;
TupleSignature::~TupleSignature() = default;

uint64_t TupleSignature::start() const {
    return startPos;
}

uint64_t TupleSignature::end() const {
    return endPos;
}

std::string TupleSignature::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "TupleSignature", verbose) + "{\n";

    if(!types.empty()) {
        result += std::string(indent, ' ') + "types: " + NodeUtils::nodeListString(sources, types, indent + 1, verbose) + "\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return result;
}

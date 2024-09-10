//
// Created by zoe on 10.09.24.
//

#include "TupleSignature.h"

uint64_t TupleSignature::start() const {
    return startPos;
}

uint64_t TupleSignature::end() const {
    return endPos;
}

std::string TupleSignature::toString(const SourceMap &sources, int indent, bool verbose) const {
    return "TODO";
}

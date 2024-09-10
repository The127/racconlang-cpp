//
// Created by zoe on 10.09.24.
//

#include "FunctionSignature.h"

uint64_t FunctionSignature::start() const {
    return startPos;
}

uint64_t FunctionSignature::end() const {
    return endPos;
}

std::string FunctionSignature::toString(const SourceMap &sources, int indent, bool verbose) const {
    return "TODO";
}

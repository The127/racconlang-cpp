//
// Created by zoe on 07.09.24.
//

#include "TypeSignature.h"

uint64_t TypeSignature::start() const {
    return startPos;
}

uint64_t TypeSignature::end() const {
    return endPos;
}

std::string TypeSignature::toString(const SourceMap &sources, int indent, bool verbose) const {
    return "TODO";
}

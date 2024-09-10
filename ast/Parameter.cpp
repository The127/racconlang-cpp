//
// Created by zoe on 10.09.24.
//

#include "Parameter.h"

uint64_t Parameter::start() const {
    return startPos;
}

uint64_t Parameter::end() const {
    return endPos;
}

std::string Parameter::toString(const SourceMap &sources, int indent, bool verbose) const {
    return "TODO";
}

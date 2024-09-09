//
// Created by zoe on 07.09.24.
//

#include "ConstraintDeclaration.h"

uint64_t ConstraintDeclaration::start() const {
    return startPos;
}

uint64_t ConstraintDeclaration::end() const {
    return endPos;
}

std::string ConstraintDeclaration::toString(const SourceMap &sources, int indent, bool verbose) const {
    return "TODO";
}

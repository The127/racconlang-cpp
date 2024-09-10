//
// Created by zoe on 07.09.24.
//

#include "EnumMemberDeclaration.h"

uint64_t EnumMemberDeclaration::start() const {
    return startPos;
}

uint64_t EnumMemberDeclaration::end() const {
    return endPos;
}

std::string EnumMemberDeclaration::toString(const SourceMap &sources, int indent, bool verbose) const {
    return "TODO";
}

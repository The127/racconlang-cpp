//
// Created by zoe on 07.09.24.
//

#include "InterfaceConstraint.h"

uint64_t InterfaceConstraint::start() const {
    return typeSignature.start();
}

uint64_t InterfaceConstraint::end() const {
    return typeSignature.end();
}

std::string InterfaceConstraint::toString(const SourceMap &sources, int indent, bool verbose) const {
    return "TODO";
}

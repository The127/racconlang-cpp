//
// Created by zoe on 07.09.24.
//

#include "ModuleVariableDeclaration.h"

uint64_t ModuleVariableDeclaration::start() const {
    return startPos;
}

uint64_t ModuleVariableDeclaration::end() const {
    return endPos;
}

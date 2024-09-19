//
// Created by zoe on 19.09.24.
//

#include "Module.h"
#include "ast/ModuleDeclaration.h"

Module::Module(const std::string_view name)
    : name(name) {
}

void Module::addPart(ModuleDeclaration &declaration) {
    parts.emplace_back(declaration);
}

//
// Created by zoe on 19.09.24.
//

#include "ModuleRegistry.h"

#include "Module.h"

void ModuleRegistry::addModulePart(const ModuleDeclaration &declaration) {
    const auto key = declaration.buildPathString();

    if(!modules.contains(key)) {
        modules.emplace(key, std::string_view(key));
    }

    modules.at(key).addPart(declaration);
}

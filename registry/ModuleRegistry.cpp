//
// Created by zoe on 19.09.24.
//

#include "ModuleRegistry.h"

#include "Module.h"


ModuleRegistry::ModuleRegistry() = default;

ModuleRegistry::~ModuleRegistry() = default;

ModuleRegistry::ModuleRegistry(ModuleRegistry &&) noexcept = default;

ModuleRegistry &ModuleRegistry::operator=(ModuleRegistry &&) noexcept = default;


void ModuleRegistry::addModulePart(ModuleDeclaration&& declaration) {
    const auto key = declaration.buildPathString();

    if (!modules.contains(key)) {
        modules.emplace(key, std::string_view(key));
    }

    modules.at(key).addPart(std::move(declaration));
}


//
// Created by zoe on 19.09.24.
//

#include "ModuleRegistry.h"

#include "Module.h"


ModuleRegistry::ModuleRegistry() = default;

ModuleRegistry::~ModuleRegistry() = default;

ModuleRegistry::ModuleRegistry(ModuleRegistry &&) noexcept = default;

ModuleRegistry &ModuleRegistry::operator=(ModuleRegistry &&) noexcept = default;

Module &ModuleRegistry::addModule(std::string path) {
    if (!modules.contains(path)) {
        modules.emplace(path, Module(path));
    }
    return modules.at(path);
}

void ModuleRegistry::populate() {
    for (auto &[key, module] : modules) {
        module.populate();
    }
}

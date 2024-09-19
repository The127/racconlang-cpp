//
// Created by zoe on 19.09.24.
//


#pragma once

#include <map>

#include "sourceMap/Source.h"


class Module;
class Path;

class ModuleRegistry {
public:
    std::map<std::string, Module> modules;

    ModuleRegistry();
    ~ModuleRegistry();
    ModuleRegistry(ModuleRegistry&&) noexcept;
    ModuleRegistry& operator=(ModuleRegistry&&) noexcept;

    void addModulePart(ModuleDeclaration&& declaration);
};

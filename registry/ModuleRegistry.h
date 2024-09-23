//
// Created by zoe on 19.09.24.
//


#pragma once

#include <map>

#include "parser/Parser.h"
#include "sourceMap/Source.h"


class ModulePart;
class Module;

class ModuleRegistry {
public:
    std::map<std::string, Module> modules;

    ModuleRegistry();
    ~ModuleRegistry();
    ModuleRegistry(ModuleRegistry&&) noexcept;
    ModuleRegistry& operator=(ModuleRegistry&&) noexcept;

    void addModule(std::string path);
    Module& getModule(std::string path);
};

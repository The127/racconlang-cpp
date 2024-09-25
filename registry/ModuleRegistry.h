//
// Created by zoe on 19.09.24.
//


#pragma once

#include <map>

#include "parser/Parser.h"
#include "sourceMap/Source.h"


class Module;
class Type;

class ModuleRegistry {
public:
    std::map<std::string, Module> modules;

    ModuleRegistry();
    ModuleRegistry(const ModuleRegistry&) = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;
    ModuleRegistry(ModuleRegistry&&) noexcept;
    ModuleRegistry& operator=(ModuleRegistry&&) noexcept;
    ~ModuleRegistry();

    Module& addModule(std::string path);

    Type& lookupType(const FileUses& uses, const Path& path);

    void populate();
};

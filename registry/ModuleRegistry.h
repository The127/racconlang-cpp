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
    std::map<std::string, Module> modules{};

    void addModulePart(const ModuleDeclaration &declaration);
};

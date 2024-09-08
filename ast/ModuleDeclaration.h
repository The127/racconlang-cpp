//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>

#include "AliasDeclaration.h"
#include "EnumDeclaration.h"
#include "FunctionDeclaration.h"
#include "InterfaceDeclaration.h"
#include "ModuleVariableDeclaration.h"
#include "StructDeclaration.h"
#include "UseNode.h"


class ModuleDeclaration : Node {
public:
    std::string path;
    std::vector<UseNode> uses;
    std::vector<EnumDeclaration> enumDeclarations;
    std::vector<InterfaceDeclaration> interfaceDeclarations;
    std::vector<StructDeclaration> structDeclarations;
    std::vector<FunctionDeclaration> functionDeclarations;
    std::vector<AliasDeclaration> aliasDeclarations;
    std::vector<ModuleVariableDeclaration> moduleVariableDeclarations;
};

//
// Created by zoe on 19.09.24.
//


#pragma once
#include <string>

#include "Struct.h"
#include "Enum.h"
#include "Alias.h"
#include "ast/ModuleDeclaration.h"
#include "ast/StructDeclaration.h"
#include "parser/Parser.h"


class Module {
public:
    std::string path;
    std::vector<Struct> structs{};
    std::vector<Enum> enums{};
    std::vector<Alias> aliases{};

    explicit Module(std::string name);

    ~Module();
    Module(Module&&) noexcept;
    Module& operator=(Module&&) noexcept;

    void addStruct(const std::shared_ptr<Source> &source, std::string name, uint8_t arity, StructDeclaration &structDeclaration);
    void addEnum(const std::shared_ptr<Source> &source, std::string name, uint8_t arity, EnumDeclaration &enumDeclaration);
    void addAlias(const std::shared_ptr<Source> &source, std::string name, uint8_t arity, AliasDeclaration &aliasDeclaration);
};

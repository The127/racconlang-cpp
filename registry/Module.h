//
// Created by zoe on 19.09.24.
//


#pragma once

#include "TypeRef.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

class Struct;
class Enum;
class Alias;
class Interface;
class Source;
class StructDeclaration;
class EnumDeclaration;
class AliasDeclaration;
class InterfaceDeclaration;
class UseMap;
class Type;
class ModuleRegistry;

class Module {
public:
    std::string path;
    std::map<std::pair<std::string, uint8_t>, TypeRef, std::less<>> types;

    // std::map<std::string, Variable, std::less<>> vars;
    // std::map<std::string, FunctionGroup, std::less<>> funcs;

    explicit Module(std::string  name);
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&&) noexcept;
    Module& operator=(Module&&) noexcept;
    ~Module();

    void addStruct(const std::shared_ptr<Source> &source, StructDeclaration &decl, const std::shared_ptr<UseMap> &useMap);
    void addEnum(const std::shared_ptr<Source> &source, EnumDeclaration &decl, const std::shared_ptr<UseMap> &useMap);
    void addAlias(const std::shared_ptr<Source> &source, AliasDeclaration &decl, const std::shared_ptr<UseMap> &useMap);
    void addInterface(const std::shared_ptr<Source> &source, InterfaceDeclaration &decl, const std::shared_ptr<UseMap> &useMap);

    std::optional<TypeRef> getType(const std::string& name, uint8_t arity);

    void populate(ModuleRegistry& registry);
    void populateStructs();
};

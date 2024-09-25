//
// Created by zoe on 19.09.24.
//


#pragma once
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
class FileUses;
class Type;

class Module {
public:
    std::string path;
    std::map<std::pair<std::string, uint8_t>, std::shared_ptr<Type>> types;

    // std::map<std::string, Variable> vars;
    // std::map<std::string, FunctionGroup> funcs;

    explicit Module(std::string  name);
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&&) noexcept;
    Module& operator=(Module&&) noexcept;
    ~Module();

    void addStruct(const std::shared_ptr<Source> &source, StructDeclaration &decl, std::shared_ptr<FileUses> &fileUses);
    void addEnum(const std::shared_ptr<Source> &source, EnumDeclaration &decl, const std::shared_ptr<FileUses> &fileUses);
    void addAlias(const std::shared_ptr<Source> &source, AliasDeclaration &decl, const std::shared_ptr<FileUses> &fileUses);
    void addInterface(const std::shared_ptr<Source> &source, InterfaceDeclaration &decl, const std::shared_ptr<FileUses> &fileUses);

    Type getType(const std::string& name, uint8_t arity);

    void populate();
    void populateStructs();
};

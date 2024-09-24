//
// Created by zoe on 19.09.24.
//


#pragma once
#include <string>
#include <vector>
#include <memory>

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

class Module {
public:
    std::string path;
    std::vector<Struct> structs;
    std::vector<Enum> enums;
    std::vector<Alias> aliases;
    std::vector<Interface> interfaces;

    explicit Module(std::string  name);
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&&) noexcept;
    Module& operator=(Module&&) noexcept;
    ~Module();

    void addStruct(const std::shared_ptr<Source> &source, std::string name, uint8_t arity,
                   StructDeclaration &structDeclaration,
                   std::shared_ptr<FileUses> &fileUses);
    void addEnum(const std::shared_ptr<Source> &source, std::string name, uint8_t arity,
                 EnumDeclaration &enumDeclaration,
                 const std::shared_ptr<FileUses> &fileUses);
    void addAlias(const std::shared_ptr<Source> &source, std::string name, uint8_t arity,
                  AliasDeclaration &aliasDeclaration,
                  const std::shared_ptr<FileUses> &fileUses);
    void addInterface(const std::shared_ptr<Source> &source, std::string name, uint8_t arity,
                      InterfaceDeclaration &interfaceDeclaration,
                      const std::shared_ptr<FileUses> &fileUses);

    void populate();
    void populateStructs();
};

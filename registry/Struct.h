//
// Created by zoe on 22.09.24.
//


#pragma once

#include <cstdint>
#include <memory>
#include <string>

class StructDeclaration;
class Source;
class FileUses;

class Struct {
public:
    std::string name;
    uint8_t arity;
    StructDeclaration* declaration;
    std::shared_ptr<Source> source;
    std::shared_ptr<FileUses> fileUses;

    Struct(std::string name, uint8_t arity, StructDeclaration* declaration, std::shared_ptr<Source> source, std::shared_ptr<FileUses> fileUses);
    Struct(const Struct&) = delete;
    Struct& operator=(const Struct&) = delete;
    Struct(Struct&&) noexcept;
    Struct& operator=(Struct&&) noexcept;
    ~Struct();
};

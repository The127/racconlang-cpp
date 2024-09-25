//
// Created by zoe on 23.09.24.
//


#pragma once

#include <cstdint>
#include <string>
#include <memory>


class EnumDeclaration;
class FileUses;
class Source;

class Enum {
public:
    std::string name;
    uint8_t arity;
    EnumDeclaration* declaration;
    std::shared_ptr<Source> source;
    std::shared_ptr<FileUses> fileUses;

    Enum(std::string name, uint8_t arity, EnumDeclaration* declaration, std::shared_ptr<Source> source, std::shared_ptr<FileUses> fileUses);
    Enum(const Enum&) = delete;
    Enum& operator=(const Enum&) = delete;
    Enum(Enum&&) noexcept;
    Enum& operator=(Enum&&) noexcept;
    ~Enum();

    void populate();
};

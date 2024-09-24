//
// Created by zoe on 23.09.24.
//


#pragma once
#include <cstdint>
#include <memory>
#include <string>


class Source;
class AliasDeclaration;
class FileUses;

class Alias {
public:
    std::string name;
    uint8_t arity;
    AliasDeclaration* declaration;
    std::shared_ptr<Source> source;
    std::shared_ptr<FileUses> fileUses;

    Alias(std::string name, uint8_t arity, AliasDeclaration* declaration, std::shared_ptr<Source> source, std::shared_ptr<FileUses> fileUses);
    Alias(const Alias&) = delete;
    Alias& operator=(const Alias&) = delete;
    Alias(Alias&&) noexcept;
    Alias& operator=(Alias&&) noexcept;
    ~Alias();

};

//
// Created by zoe on 23.09.24.
//


#pragma once

#include <cstdint>
#include <string>
#include <memory>


class InterfaceDeclaration;
class FileUses;
class Source;

class Interface {
public:
    std::string name;
    uint8_t arity;
    InterfaceDeclaration *declaration;
    std::shared_ptr<Source> source;
    std::shared_ptr<FileUses> fileUses;

    Interface(std::string name, uint8_t arity, InterfaceDeclaration *declaration, std::shared_ptr<Source> source,
              std::shared_ptr<FileUses> fileUses);
    ~Interface();
    Interface(Interface &&) noexcept;
    Interface &operator=(Interface &&) noexcept;

    void populate();
};

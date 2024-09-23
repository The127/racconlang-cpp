//
// Created by zoe on 23.09.24.
//


#pragma once

#include <cstdint>
#include <string>

#include "ast/FileUses.h"


class InterfaceDeclaration;

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
};
//
// Created by zoe on 23.09.24.
//


#pragma once

#include <cstdint>
#include <string>


class InterfaceDeclaration;

class Interface {
public:
    std::string name;
    uint8_t arity;
    InterfaceDeclaration* declaration;

    Interface(std::string name, uint8_t arity, InterfaceDeclaration* declaration);
    ~Interface();
    Interface(Interface&&) noexcept;
    Interface& operator=(Interface&&) noexcept;
};

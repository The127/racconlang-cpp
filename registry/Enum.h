//
// Created by zoe on 23.09.24.
//


#pragma once
#include <cstdint>
#include <string>

class EnumDeclaration;

class Enum {
public:
    std::string name;
    uint8_t arity;
    EnumDeclaration* declaration;

    Enum(std::string name, uint8_t arity, EnumDeclaration* declaration);
    ~Enum();
    Enum(Enum&&) noexcept;
    Enum& operator=(Enum&&) noexcept;
};

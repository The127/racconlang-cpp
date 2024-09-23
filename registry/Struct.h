//
// Created by zoe on 22.09.24.
//


#pragma once

#include <cstdint>
#include <string>

class StructDeclaration;

class Struct {
public:
    std::string name;
    uint8_t arity;
    StructDeclaration* declaration;

    Struct(std::string name, uint8_t arity, StructDeclaration* declaration);
    ~Struct();
    Struct(Struct&&) noexcept;
    Struct& operator=(Struct&&) noexcept;
};

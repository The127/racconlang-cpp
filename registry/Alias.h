//
// Created by zoe on 23.09.24.
//


#pragma once
#include <cstdint>
#include <string>


class AliasDeclaration;

class Alias {
public:
    std::string name;
    uint8_t arity;
    AliasDeclaration* declaration;

    Alias(std::string name, uint8_t arity, AliasDeclaration* declaration);
    ~Alias();
    Alias(Alias&&) noexcept;
    Alias& operator=(Alias&&) noexcept;

};

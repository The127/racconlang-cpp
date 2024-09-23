//
// Created by zoe on 23.09.24.
//

#include "Enum.h"

Enum::Enum(std::string name, uint8_t arity, EnumDeclaration *declaration)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration) {
}

Enum::~Enum() = default;
Enum::Enum(Enum &&) noexcept = default;
Enum &Enum::operator=(Enum &&) noexcept = default;

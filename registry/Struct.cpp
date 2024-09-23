//
// Created by zoe on 22.09.24.
//

#include "Struct.h"

#include <utility>

Struct::Struct(std::string name, const uint8_t arity, StructDeclaration *declaration)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration) {
}

Struct::~Struct() = default;
Struct::Struct(Struct &&) noexcept = default;
Struct & Struct::operator=(Struct &&) noexcept = default;

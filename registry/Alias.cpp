//
// Created by zoe on 23.09.24.
//

#include "Alias.h"

#include <utility>

Alias::Alias(std::string name, uint8_t arity, AliasDeclaration *declaration)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration) {
}

Alias::~Alias() = default;
Alias::Alias(Alias &&) noexcept = default;
Alias &Alias::operator=(Alias &&) noexcept = default;

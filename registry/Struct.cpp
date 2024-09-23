//
// Created by zoe on 22.09.24.
//

#include "Struct.h"

#include <utility>

Struct::Struct(std::string name, const uint8_t arity, StructDeclaration *declaration, std::shared_ptr<Source> source,
               std::shared_ptr<FileUses> fileUses)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration),
      source(std::move(source)),
      fileUses(std::move(fileUses)) {
}

Struct::~Struct() = default;
Struct::Struct(Struct &&) noexcept = default;
Struct &Struct::operator=(Struct &&) noexcept = default;

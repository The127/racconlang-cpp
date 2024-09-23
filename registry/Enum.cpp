//
// Created by zoe on 23.09.24.
//

#include "Enum.h"

#include <utility>

Enum::Enum(std::string name, uint8_t arity, EnumDeclaration *declaration, std::shared_ptr<Source> source,
           std::shared_ptr<FileUses> fileUses)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration),
      source(std::move(source)),
      fileUses(std::move(fileUses)) {
}

Enum::~Enum() = default;
Enum::Enum(Enum &&) noexcept = default;
Enum &Enum::operator=(Enum &&) noexcept = default;

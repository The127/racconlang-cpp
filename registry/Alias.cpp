//
// Created by zoe on 23.09.24.
//

#include "Alias.h"

#include "ast/AliasDeclaration.h"
#include "ast/FileUses.h"
#include "sourceMap/Source.h"

#include <utility>

Alias::Alias(std::string name, uint8_t arity, AliasDeclaration *declaration, std::shared_ptr<Source> source,
             std::shared_ptr<FileUses> fileUses)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration),
      source(std::move(source)),
      fileUses(std::move(fileUses)) {
}

void Alias::populate() {

}

Alias::~Alias() = default;
Alias::Alias(Alias &&) noexcept = default;
Alias &Alias::operator=(Alias &&) noexcept = default;

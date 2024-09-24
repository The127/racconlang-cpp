//
// Created by zoe on 23.09.24.
//

#include "Interface.h"

#include "ast/FileUses.h"
#include "ast/InterfaceDeclaration.h"
#include "sourceMap/Source.h"

Interface::Interface(std::string name, uint8_t arity, InterfaceDeclaration *declaration, std::shared_ptr<Source> source,
                     std::shared_ptr<FileUses> fileUses)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration),
      source(std::move(source)),
      fileUses(std::move(fileUses)) {
}

Interface::~Interface() = default;
Interface::Interface(Interface &&) noexcept = default;
Interface &Interface::operator=(Interface &&) noexcept = default;

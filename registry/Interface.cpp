//
// Created by zoe on 23.09.24.
//

#include "Interface.h"

Interface::Interface(std::string name, uint8_t arity, InterfaceDeclaration *declaration)
    : name(std::move(name)),
      arity(arity),
      declaration(declaration) {
}

Interface::~Interface() = default;
Interface::Interface(Interface &&) noexcept = default;
Interface &Interface::operator=(Interface &&) noexcept = default;

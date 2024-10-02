//
// Created by zoe on 23.09.24.
//

#include "Interface.h"

#include "TypeRefImpl.h"
#include "ast/UseMap.h"
#include "ast/InterfaceDeclaration.h"
#include "sourceMap/Source.h"

Interface::Interface(std::string name, std::string_view module, uint8_t arity, InterfaceDeclaration *declaration, std::shared_ptr<Source> source,
                     std::shared_ptr<UseMap> useMap)
        : name(std::move(name)),
          modulePath(module),
          arity(arity),
          declaration(declaration),
          source(std::move(source)),
          useMap(std::move(useMap)) {
}

void Interface::populate(ModuleRegistry &registry) {
    // TODO
}

TypeRef Interface::concretize(ModuleRegistry &registry, const std::vector<TypeRef>& args) const {
    // TODO
    return TypeRef::unknown();
}

Interface::~Interface() = default;

Interface::Interface(Interface &&) noexcept = default;

Interface &Interface::operator=(Interface &&) noexcept = default;

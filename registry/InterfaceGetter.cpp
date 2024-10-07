#include "InterfaceGetter.h"
#include "Interface.h"
#include "ast/InterfaceGetterDeclaration.h"

namespace racc::registry {
    InterfaceGetter InterfaceGetter::make(ModuleRegistry &registry, ast::InterfaceGetterDeclaration &decl, Interface* interface) {
        InterfaceGetter getter;
        getter.decl = &decl;
        getter.interface = interface;

        getter.mut = decl.isMut;

        if (decl.name) {
            getter.name = *decl.name;
        }

        if (decl.returnType) {
            auto typeRes = registry.lookupType(decl.returnType->type, interface->genericParamsMap, interface->modulePath, *interface->useMap);
            if (typeRes) {
                getter.type = *typeRes;
            } else {
                interface->source->addError(std::move(typeRes.error()));
            }

            getter.retMut = decl.returnType->isMut;
        }

        return getter;
    }

    InterfaceGetter::InterfaceGetter() : type(TypeRef::unknown()) {}

    InterfaceGetter::~InterfaceGetter() = default;

    InterfaceGetter &InterfaceGetter::operator=(InterfaceGetter &&) noexcept = default;

    InterfaceGetter::InterfaceGetter(InterfaceGetter &&) noexcept = default;
}
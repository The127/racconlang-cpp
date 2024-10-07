#include "InterfaceSetter.h"

#include "ast/InterfaceSetterDeclaration.h"
#include "ModuleRegistry.h"
#include "Interface.h"

namespace racc::registry {

    InterfaceSetter InterfaceSetter::make(ModuleRegistry &registry, ast::InterfaceSetterDeclaration &decl, Interface* interface) {
        InterfaceSetter setter{};
        setter.decl = &decl;
        setter.interface = interface;

        if (decl.name) {
            setter.name = decl.name->name;
        }

        if (decl.parameter) {
            if (decl.parameter->type) {
                auto typeRes = registry.lookupType(*decl.parameter->type, interface->genericParamsMap, interface->modulePath, *interface->useMap);
                if (typeRes) {
                    setter.type = *typeRes;
                } else {
                    interface->source->addError(std::move(typeRes.error()));
                }
            }

            setter.argMut = decl.parameter->isMut;
            setter.argName = decl.parameter->name.name;
        }

        return setter;
    }

    InterfaceSetter::InterfaceSetter() : type(TypeRef::unknown()) {}

    InterfaceSetter::~InterfaceSetter() = default;

    InterfaceSetter &InterfaceSetter::operator=(InterfaceSetter &&) noexcept = default;

    InterfaceSetter::InterfaceSetter(InterfaceSetter &&) noexcept = default;
}
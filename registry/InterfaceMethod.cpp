#include "InterfaceMethod.h"

#include "Parameter.h"
#include "ast/InterfaceMethodDeclaration.h"
#include "ast/Parameter.h"
#include "Interface.h"

namespace racc::registry {


    InterfaceMethod::InterfaceMethod() : returnType(TypeRef::unknown()) {
    }

    InterfaceMethod InterfaceMethod::make(
            ModuleRegistry &registry,
            ast::InterfaceMethodDeclaration &decl,
            Interface &interface
    ) {
        InterfaceMethod method;
        method.decl = &decl;
        method.interface = interface.ptr();
        method.isMut = decl.isMut;

        method.genericParamsMap = interface.genericParamsMap;
        for (const auto &param: decl.genericParams) {
            auto p = method.genericParams.emplace_back(TypeRef::var(param));
            method.genericParamsMap.insert_or_assign(param, p);
        }

        for (const auto &paramDecl: decl.parameters) {
            auto paramType = TypeRef::unknown();
            if (paramDecl.type) {
                auto typeRes = registry.lookupType(*paramDecl.type, method.genericParamsMap, interface.modulePath, *interface.useMap);
                if (typeRes) {
                    paramType = *typeRes;
                } else {
                    interface.source->addError(std::move(typeRes.error()));
                }
            }

            auto paramMode = paramDecl.isRef ? ParameterMode::Ref : (paramDecl.isMut ? ParameterMode::Mut : ParameterMode::Normal);

            method.params.emplace_back(paramDecl.name, paramType, paramMode);
        }

        if (decl.returnType) {
            method.returnMut = decl.returnType->isMut;
            auto typeRes = registry.lookupType(decl.returnType->type, method.genericParamsMap, interface.modulePath, *interface.useMap);
            if (typeRes) {
                method.returnType = *typeRes;
            } else {
                interface.source->addError(std::move(typeRes.error()));
            }
        } else {
            method.returnType = TypeRef::empty();
        }

        if (decl.name) {
            method.name = Id(*decl.name);
        }

        return method;
    }

    InterfaceMethod &InterfaceMethod::operator=(InterfaceMethod &&) noexcept = default;

    InterfaceMethod::InterfaceMethod(InterfaceMethod &&) noexcept = default;

    InterfaceMethod::~InterfaceMethod() = default;

}

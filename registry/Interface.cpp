//
// Created by zoe on 23.09.24.
//

#include "Interface.h"

#include "TypeRef.h"
#include "ast/UseMap.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/InterfaceMethodDeclaration.h"
#include "ast/InterfaceGetterDeclaration.h"
#include "ast/InterfaceSetterDeclaration.h"
#include "sourceMap/Source.h"
#include "InterfaceMethod.h"
#include "InterfaceGetter.h"
#include "InterfaceSetter.h"
#include "Parameter.h"

namespace racc::registry {
    Interface::Interface(std::string name, std::string_view module, uint8_t arity, ast::InterfaceDeclaration *declaration,
                         std::shared_ptr<sourcemap::Source> source,
                         std::shared_ptr<ast::UseMap> useMap)
            : name(std::move(name)),
              modulePath(module),
              arity(arity),
              isPublic(declaration->isPublic),
              declaration(declaration),
              source(std::move(source)),
              useMap(std::move(useMap)) {

        for (const auto &item: declaration->genericParams) {
            auto &t = genericParams.emplace_back(TypeRef::var(std::string(item.name)));
            const auto &[_, success] = genericParamsMap.emplace(item.name, t);
            COMPILER_ASSERT(success, "insert into genericParamsMap failed");
        }
    }

    void Interface::populate(ModuleRegistry &registry) {
        for (auto &methodDecl: declaration->methods) {
            // the method is constructed here before the check for the name, so that types are checked regardless
            auto method = InterfaceMethod::make(registry, methodDecl, *this);

            if (!methodDecl.name) continue;

            auto [res, ok] = methods.emplace(method.name, std::move(method));
            COMPILER_ASSERT(ok, "failed to insert method");
        }

        for (auto &setterDecl : declaration->setters) {
            // the setter is constructed here before the check for the name, so that types are checked regardless
            auto setter = InterfaceSetter::make(registry, setterDecl, this);

            if (!setterDecl.name) continue;

            auto [res, ok] = setters.emplace(setter.name, std::move(setter));
            COMPILER_ASSERT(ok, "failed to insert setter");
        }

        for (auto &getterDecl : declaration->getters) {
            // the setter is constructed here before the check for the name, so that types are checked regardless
            auto getter = InterfaceGetter::make(registry, getterDecl, this);

            if (!getterDecl.name) continue;

            auto [res, ok] = getters.emplace(getter.name, std::move(getter));
            COMPILER_ASSERT(ok, "failed to insert getter");
        }
    }

    TypeRef Interface::concretize(ModuleRegistry &registry, const std::vector<TypeRef> &args) const {
        // TODO
        return TypeRef::unknown();
    }

    Interface::~Interface() = default;

    Interface::Interface(Interface &&) noexcept = default;

    Interface &Interface::operator=(Interface &&) noexcept = default;

}

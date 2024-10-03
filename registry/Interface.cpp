//
// Created by zoe on 23.09.24.
//

#include "Interface.h"

#include "TypeRefImpl.h"
#include "ast/UseMap.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/InterfaceMethodDeclaration.h"
#include "ast/InterfaceGetterDeclaration.h"
#include "ast/InterfaceSetterDeclaration.h"
#include "sourceMap/Source.h"
#include "InterfaceMethod.h"
#include "InterfaceGetter.h"
#include "InterfaceSetter.h"

namespace racc::registry {

    Interface::Interface(std::string name, std::string_view module, uint8_t arity, ast::InterfaceDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
                         std::shared_ptr<ast::UseMap> useMap)
            : name(std::move(name)),
              modulePath(module),
              arity(arity),
              declaration(declaration),
              source(std::move(source)),
              useMap(std::move(useMap)),
              isPublic(declaration->isPublic) {

        for (const auto &item: declaration->genericParams) {
            auto &t = genericParams.emplace_back(TypeRef::var(std::string(item.name)));
            const auto &[_, success] = genericParamsMap.emplace(item.name, t);
            COMPILER_ASSERT(success, "insert into genericParamsMap failed");
        }
    }

    void Interface::populate(ModuleRegistry &registry) {
        for (const auto &item: declaration->methods) {
            if (!item.name) continue;

            auto methodName = std::string(item.name->name);
            auto [res, ok] = methods.emplace(methodName, InterfaceMethod());
            auto &method = res->second;
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

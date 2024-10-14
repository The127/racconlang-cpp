//
// Created by zoe on 19.09.24.
//

#include "Module.h"

#include <utility>
#include <ast/ImplBlock.h>

#include "TypeRef.h"
#include "ast/EnumDeclaration.h"
#include "ast/AliasDeclaration.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/StructDeclaration.h"
#include "sourceMap/Source.h"
#include "parser/Parser.h"
#include "registry/ModuleRegistry.h"
#include "sourceMap/Source.h"
#include "Struct.h"
#include "Interface.h"
#include "Enum.h"
#include "TupleType.h"
#include "Alias.h"

namespace racc::registry {

    Module::Module(Id path)
            : path(path) {
    }

    Module::~Module() = default;

    Module::Module(Module &&) noexcept = default;

    Module &Module::operator=(Module &&) noexcept = default;

    void Module::addStruct(const std::shared_ptr<sourcemap::Source> &source, ast::StructDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap) {
        COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
        auto name = Id(*decl.name);
        auto arity = decl.genericParams.size();
        if (types.contains({name, arity})) {
            source->errors.emplace_back(errors::ErrorCode::DuplicateTypeDeclaration, decl.start());
            return;
        }

        types.emplace(std::make_pair(name, arity), TypeRef::makeStruct(name, path, arity, &decl, source, useMap).first);
    }

    void Module::addEnum(const std::shared_ptr<sourcemap::Source> &source, ast::EnumDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap) {
        COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
        auto name = Id(*decl.name);
        auto arity = decl.genericParams.size();
        if (types.contains({name, arity})) {
            source->errors.emplace_back(errors::ErrorCode::DuplicateTypeDeclaration, decl.start());
            return;
        }

        types.emplace(std::make_pair(name, arity), TypeRef::makeEnum(name, path, arity, &decl, source, useMap).first);
    }

    void Module::addAlias(const std::shared_ptr<sourcemap::Source> &source, ast::AliasDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap) {
        COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
        auto name = Id(*decl.name);
        auto arity = decl.genericParams.size();
        if (types.contains({name, arity})) {
            source->errors.emplace_back(errors::ErrorCode::DuplicateTypeDeclaration, decl.start());
            return;
        }

        types.emplace(std::make_pair(name, arity), TypeRef::makeAlias(name, path, arity, &decl, source, useMap).first);
    }

    void Module::addInterface(const std::shared_ptr<sourcemap::Source> &source, ast::InterfaceDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap) {
        COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
        auto name = Id(*decl.name);
        auto arity = decl.genericParams.size();
        if (types.contains({name, arity})) {
            source->errors.emplace_back(errors::ErrorCode::DuplicateTypeDeclaration, decl.start());
            return;
        }

        types.emplace(std::make_pair(name, arity), TypeRef::makeInterface(name, path, arity, &decl, source, useMap).first);
    }

    void addImplBlock(const std::shared_ptr<sourcemap::Source> &source, ast::ImplBlock &decl, const std::shared_ptr<ast::UseMap> &useMap){
        COMPILER_ASSERT(decl.structName.has_value(), "trued to add impl block without struct name");
        //TODO: do the things
    }

    void Module::populate(ModuleRegistry &registry) {
        for (auto &[key, type]: types) {
            auto &[name, arity] = key;
            type.populate(registry);
        }
        //TODO: anything else?
    }

    void Module::populateStructs() {
//    for (auto &s : structs) {
//        s.declaration->genericParams
//    }
    }

    std::optional<TypeRef> Module::getType(const Id &name, uint8_t arity) {
        auto it = types.find({name, arity});
        if (it == types.end()) {
            return std::nullopt;
        }

        return it->second;
    }

}

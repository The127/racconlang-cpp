//
// Created by zoe on 19.09.24.
//


#pragma once

#include "predeclare.h"

#include "Id.h"
#include "TypeRef.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

class racc::registry::Module {
public:
    Id path;
    std::map<std::pair<Id, uint8_t>, TypeRef, std::less<>> types;

    // std::map<Id, Variable, std::less<>> vars;
    // std::map<Id, FunctionGroup, std::less<>> funcs;

    explicit Module(Id name);

    Module(const Module &) = delete;

    Module &operator=(const Module &) = delete;

    Module(Module &&) noexcept;

    Module &operator=(Module &&) noexcept;

    ~Module();

    void addStruct(const std::shared_ptr<sourcemap::Source> &source, ast::StructDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap);

    void addEnum(const std::shared_ptr<sourcemap::Source> &source, ast::EnumDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap);

    void addAlias(const std::shared_ptr<sourcemap::Source> &source, ast::AliasDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap);

    void addInterface(const std::shared_ptr<sourcemap::Source> &source, ast::InterfaceDeclaration &decl, const std::shared_ptr<ast::UseMap> &useMap);

    void addImplBlock(const std::shared_ptr<sourcemap::Source> &source, ast::ImplBlock &decl, const std::shared_ptr<ast::UseMap> &useMap);

    std::optional<TypeRef> getType(const Id &name, uint8_t arity);

    void populate(ModuleRegistry &registry);

    void populateStructs();
};

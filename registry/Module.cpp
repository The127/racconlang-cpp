//
// Created by zoe on 19.09.24.
//

#include "Module.h"

#include <utility>

#include "TypeRefImpl.h"
#include "ast/EnumDeclaration.h"
#include "ast/AliasDeclaration.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/StructDeclaration.h"
#include "sourceMap/Source.h"
#include "parser/Parser.h"
#include "registry/ModuleRegistry.h"


Module::Module(std::string path)
        : path(std::move(path)) {
}

Module::~Module() = default;

Module::Module(Module &&) noexcept = default;

Module &Module::operator=(Module &&) noexcept = default;

void Module::addStruct(const std::shared_ptr<Source> &source, StructDeclaration &decl, const std::shared_ptr<UseMap> &useMap) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), TypeRef::make<Struct>(name, path, arity, &decl, source, useMap));
}

void Module::addEnum(const std::shared_ptr<Source> &source, EnumDeclaration &decl, const std::shared_ptr<UseMap> &useMap) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), TypeRef::make<Enum>(name, path, arity, &decl, source, useMap));
}

void Module::addAlias(const std::shared_ptr<Source> &source, AliasDeclaration &decl, const std::shared_ptr<UseMap> &useMap) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), TypeRef::make<Alias>(name, path, arity, &decl, source, useMap));
}

void Module::addInterface(const std::shared_ptr<Source> &source, InterfaceDeclaration &decl, const std::shared_ptr<UseMap> &useMap) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), TypeRef::make<Interface>(name, path, arity, &decl, source, useMap));
}

void Module::populate(ModuleRegistry& registry) {
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

std::optional<TypeRef> Module::getType(const std::string &name, uint8_t arity) {
    auto it = types.find({name, arity});
    if (it == types.end()) {
        return std::nullopt;
    }

    return it->second;
}

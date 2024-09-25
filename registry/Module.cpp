//
// Created by zoe on 19.09.24.
//

#include "Module.h"

#include <utility>

#include "Type.h"
#include "Struct.h"
#include "Enum.h"
#include "Alias.h"
#include "Interface.h"
#include "ast/EnumDeclaration.h"
#include "ast/AliasDeclaration.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/StructDeclaration.h"
#include "sourceMap/Source.h"
#include "parser/Parser.h"


Module::Module(std::string path)
        : path(std::move(path)) {
}

Module::~Module() = default;

Module::Module(Module &&) noexcept = default;

Module &Module::operator=(Module &&) noexcept = default;

void Module::addStruct(const std::shared_ptr<Source> &source, StructDeclaration &decl, std::shared_ptr<FileUses> &fileUses) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), std::make_shared<Type>(Struct(name, arity, &decl, source, fileUses)));
}

void Module::addEnum(const std::shared_ptr<Source> &source, EnumDeclaration &decl, const std::shared_ptr<FileUses> &fileUses) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), std::make_shared<Type>(Enum(name, arity, &decl, source, fileUses)));
}

void Module::addAlias(const std::shared_ptr<Source> &source, AliasDeclaration &decl, const std::shared_ptr<FileUses> &fileUses) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), std::make_shared<Type>(Alias(name, arity, &decl, source, fileUses)));
}

void Module::addInterface(const std::shared_ptr<Source> &source, InterfaceDeclaration &decl, const std::shared_ptr<FileUses> &fileUses) {
    COMPILER_ASSERT(decl.name.has_value(), "tried to add unnamed struct to registry");
    auto name = std::string(decl.name->name);
    auto arity = decl.genericParams.size();
    if (types.contains({name, arity})) {
        source->errors.emplace_back(ErrorCode::DuplicateTypeDeclaration, decl.start());
        return;
    }

    types.emplace(std::make_pair(name, arity), std::make_shared<Type>(Interface(name, arity, &decl, source, fileUses)));
}

void Module::populate() {
    for (auto &[key, type]: types) {
        auto &[name, arity] = key;
        type->populate();
    }
    //TODO: rest
}

void Module::populateStructs() {
//    for (auto &s : structs) {
//        s.declaration->genericParams
//    }
}

Type Module::getType(const std::string &name, uint8_t arity) {
//    auto it = std::ranges::find_if(structs, [&](Struct &s) {
//        return s.name == name && s.arity == arity;
//    });
}

//
// Created by zoe on 19.09.24.
//


#pragma once

#include "predeclare.h"

#include "Id.h"
#include "parser/Parser.h"
#include "sourceMap/Source.h"
#include "FunctionType.h"
#include "Module.h"

#include <map>




class racc::registry::ModuleRegistry {

public:
    std::map<Id, Module, std::less<>> modules;

    std::map<std::vector<TypeRef>, TypeRef> tupleTypes;
    std::map<std::vector<std::pair<ParameterMode, TypeRef>>, TypeRef> functionTypes;

    std::map<std::pair<TypeRef, std::vector<TypeRef>>, TypeRef> genericTypes;

    std::map<Id, TypeRef, std::less<>> builtinTypes;

    ModuleRegistry();

    ModuleRegistry(const ModuleRegistry &) = delete;

    ModuleRegistry &operator=(const ModuleRegistry &) = delete;

    ModuleRegistry(ModuleRegistry &&) noexcept;

    ModuleRegistry &operator=(ModuleRegistry &&) noexcept;

    ~ModuleRegistry();

    [[nodiscard]] Module &addModule(Id path);

    [[nodiscard]] Module &getModule(Id path);

    [[nodiscard]] const Module &getModule(Id path) const;

    std::expected<TypeRef, errors::CompilerError>
    lookupType(const ast::Signature &signature, const std::map<Id, TypeRef, std::less<>> &generics, Id moduleName,
               const ast::UseMap &uses);

    TypeRef getTupleType(const std::vector<TypeRef> &types);

    TypeRef
    getFunctionType(const std::vector<std::pair<ParameterMode, TypeRef>> &argTypes, const TypeRef &returnType, bool returnMut);

    TypeRef
    getGenericType(const TypeRef &type, const std::vector<TypeRef> &genericArguments);

    void populate();


private:
    std::expected<TypeRef, errors::CompilerError>
    lookupTupleType(const ast::TupleSignature &signature, const std::map<Id, TypeRef, std::less<>> &generics, Id moduleName,
                    const ast::UseMap &uses);

    std::expected<TypeRef, errors::CompilerError>
    lookupFunctionType(const ast::FunctionSignature &signature, const std::map<Id, TypeRef, std::less<>> &generics, Id moduleName,
                       const ast::UseMap &uses);

    std::expected<TypeRef, errors::CompilerError>
    lookupNamedType(const ast::TypeSignature &signature, const std::map<Id, TypeRef, std::less<>> &generics, Id currentModuleName,
                    const ast::UseMap &uses);

};

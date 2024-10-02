//
// Created by zoe on 19.09.24.
//


#pragma once

#include <map>

#include "parser/Parser.h"
#include "sourceMap/Source.h"
#include "FunctionType.h"


class Module;

class Type;

bool tuple_typelist_compare(const std::vector<TypeRef> &a, const std::vector<TypeRef> &b);


bool function_typelist_compare(const std::vector<std::pair<ParameterMode, TypeRef>> &a,
                               const std::vector<std::pair<ParameterMode, TypeRef>> &b);


class ModuleRegistry {
public:
    std::map<std::string, Module, std::less<>> modules;

    std::map<std::vector<TypeRef>, TypeRef, decltype(tuple_typelist_compare) *> tupleTypes;
    std::map<std::vector<std::pair<ParameterMode, TypeRef>>, TypeRef, decltype(function_typelist_compare) *> functionTypes;

    std::map<std::pair<TypeRef, std::vector<TypeRef>>, TypeRef> genericTypes;

    std::map<std::string, TypeRef, std::less<>> builtinTypes;

    ModuleRegistry();

    ModuleRegistry(const ModuleRegistry &) = delete;

    ModuleRegistry &operator=(const ModuleRegistry &) = delete;

    ModuleRegistry(ModuleRegistry &&) noexcept;

    ModuleRegistry &operator=(ModuleRegistry &&) noexcept;

    ~ModuleRegistry();

    Module &addModule(std::string path);

    template<typename T>
    Module& getModule(T path) {
        auto it = modules.find(path);
        COMPILER_ASSERT(it != modules.end(), "unknown module");
        return it->second;
    }

    template<typename T>
    const Module& getModule(T path) const {
        auto it = modules.find(path);
        COMPILER_ASSERT(it != modules.end(), "unknown module");
        return it->second;
    }

    std::expected<TypeRef, CompilerError>
    lookupType(const Signature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view moduleName, const UseMap &uses);

    TypeRef getTupleType(const std::vector<TypeRef> &types);

    TypeRef
    getFunctionType(const std::vector<std::pair<ParameterMode, TypeRef>> &argTypes, const TypeRef &returnType, bool returnMut);

    TypeRef
    getGenericType(const TypeRef &type, const std::vector<TypeRef> &genericArguments);

    void populate();


private:
    std::expected<TypeRef, CompilerError>
    lookupTupleType(const TupleSignature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view moduleName, const UseMap &uses);

    std::expected<TypeRef, CompilerError>
    lookupFunctionType(const FunctionSignature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view moduleName, const UseMap &uses);

    std::expected<TypeRef, CompilerError>
    lookupNamedType(const TypeSignature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view currentModuleName, const UseMap &uses);

};

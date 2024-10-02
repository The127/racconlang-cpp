//
// Created by zoe on 19.09.24.
//

#include <ranges>
#include "ModuleRegistry.h"

#include "TypeRefImpl.h"
#include "Module.h"
#include "TupleType.h"
#include "ast/Signature.h"
#include "ast/NamelessParameter.h"
#include "ast/ReturnType.h"
#include "ast/UseMap.h"
#include "utils/StringUtils.h"


ModuleRegistry::ModuleRegistry() : tupleTypes(tuple_typelist_compare), functionTypes(function_typelist_compare) {
    builtinTypes.emplace("int", TypeRef::builtin("int", 4)); // TODO
}

ModuleRegistry::~ModuleRegistry() = default;

ModuleRegistry::ModuleRegistry(ModuleRegistry &&) noexcept = default;

ModuleRegistry &ModuleRegistry::operator=(ModuleRegistry &&) noexcept = default;

Module &ModuleRegistry::addModule(std::string path) {
    if (!modules.contains(path)) {
        modules.emplace(path, Module(path));
    }
    return modules.at(path);
}

void ModuleRegistry::populate() {
    for (auto &[key, module]: modules) {
        module.populate(*this);
    }
}

TypeRef ModuleRegistry::getTupleType(const std::vector<TypeRef> &types) {
    auto [it, inserted] = tupleTypes.emplace(types, TypeRef::make<TupleType>(TupleType(types)));
    if (!inserted) {
        it = tupleTypes.find(types);
    }
    COMPILER_ASSERT(it != tupleTypes.end(), "tuple type failed to insert but also failed to find");
    return it->second;
}

TypeRef
ModuleRegistry::getFunctionType(const std::vector<std::pair<ParameterMode, TypeRef>> &argTypes, const TypeRef &returnType,
                                bool returnMut) {
    auto types = argTypes;
    types.emplace_back(returnMut ? ParameterMode::Mut : ParameterMode::Normal, returnType);
    auto [it, inserted] = functionTypes.emplace(types, TypeRef::make<FunctionType>(argTypes, returnType, returnMut));
    if (!inserted) {
        it = functionTypes.find(types);
    }
    COMPILER_ASSERT(it != functionTypes.end(), "function type failed to insert but also failed to find");
    return it->second;
}


TypeRef ModuleRegistry::getGenericType(const TypeRef &type, const std::vector<TypeRef> &genericArguments) {
    auto it = genericTypes.find({type, genericArguments});
    if (it != genericTypes.end()) {
        return it->second;
    }

    auto [it2, inserted] = genericTypes.emplace(std::make_pair(type, genericArguments), type.concretize(*this, genericArguments));
    COMPILER_ASSERT(inserted, "function type failed to find but also failed to insert");
    return it2->second;
}


std::expected<TypeRef, CompilerError>
ModuleRegistry::lookupType(const Signature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view moduleName,
                           const UseMap &uses) {
    if (signature.isTuple()) {
        return lookupTupleType(signature.asTuple(), generics, moduleName, uses);
    }

    if (signature.isFunction()) {
        return this->lookupFunctionType(signature.asFunction(), generics, moduleName, uses);
    }

    if (signature.isType()) {
        return this->lookupNamedType(signature.asType(), generics, moduleName, uses);
    }

    COMPILER_ASSERT(false, "signature is not a tuple type, function type or named type");
}

std::expected<TypeRef, CompilerError>
ModuleRegistry::lookupTupleType(const TupleSignature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view moduleName,
                                const UseMap &uses) {
    std::vector<TypeRef> types;

    for (const auto &item: signature.types) {
        auto result = lookupType(item, generics, moduleName, uses);
        if (!result) {
            return result;
        }
        types.emplace_back(std::move(*result));

    }

    return getTupleType(types);
}

std::expected<TypeRef, CompilerError>
ModuleRegistry::lookupFunctionType(const FunctionSignature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view moduleName,
                                   const UseMap &uses) {
    std::vector<std::pair<ParameterMode, TypeRef>> params;
    for (const auto &item: signature.parameterTypes) {
        auto mode = ParameterMode::Normal;
        if (item.isRef) {
            mode = ParameterMode::Ref;
        } else if (item.isMut) {
            mode = ParameterMode::Mut;
        }
        auto result = lookupType(item.type, generics, moduleName, uses);
        if (!result) {
            return result;
        }
        params.emplace_back(mode, std::move(*result));
    }

    TypeRef returnType = TypeRef::unknown();
    auto returnMut = false;
    if (signature.returnType != nullptr) {
        auto result = lookupType(signature.returnType->type, generics, moduleName, uses);
        if (!result) {
            return result;
        }
        returnType = std::move(*result);
        returnMut = signature.returnType->isMut;
    }

    return getFunctionType(params, returnType, returnMut);
}

std::expected<TypeRef, CompilerError>
ModuleRegistry::lookupNamedType(const TypeSignature &signature, const std::map<std::string, TypeRef, std::less<>> &generics, std::string_view currentModuleName,
                                const UseMap &uses) {

    uint8_t arity = signature.genericArguments.size();

    auto lookupTypeByPath = [this](const std::string &path, uint8_t arity) -> std::optional<TypeRef> {
        auto pos = path.rfind("::");
        COMPILER_ASSERT(pos != std::string::npos, "invalid module path");

        auto moduleName = std::string_view(path).substr(0, pos);
        auto typeName = std::string_view(path).substr(pos + 2);

        auto mod = modules.find(moduleName);
        if (mod == modules.end()) {
            return std::nullopt;
        }

        auto it = mod->second.types.find(std::make_pair(typeName, arity));

        if (it == mod->second.types.end()) {
            return std::nullopt;
        }
        return it->second;
    };


    COMPILER_ASSERT(!signature.path.parts.empty(), "signature path is empty");

    TypeRef t = TypeRef::unknown();

    if (signature.path.isRooted()) {
        if (signature.path.parts.size() == 1) {
            auto &name = signature.path.parts[0].name;
            auto it = builtinTypes.find(name);
            if (it != builtinTypes.end()) {
                if (arity != 0) {
                    auto err = CompilerError(ErrorCode::UnknownType, signature.start());
                    err.addLabel(std::format("the type {} does not take any type arguments", name), signature.genericStartPos, signature.genericEndPos);
                    return std::unexpected(std::move(err));
                }

                t = it->second;
            }
        } else {
            auto path = StringUtils::join(signature.path.parts
                                          | std::ranges::views::transform([](const auto &i) { return i.name; }), "::");

            auto res = lookupTypeByPath(path, arity);
            if (res) {
                t = *res;
            }
        }
    } else {
        if (signature.path.parts.size() == 1) {
            auto name = signature.path.parts[0].name;
            {
                auto it = generics.find(name);
                if (it != generics.end()) {
                    t = it->second;
                }
            }


            auto path = uses.lookup(name);
            if (path) {
                auto res = lookupTypeByPath(*path, arity);
                if (res) {
                    t = *res;
                }
            }

            const auto &currentModule = getModule(currentModuleName);
            {
                auto it = currentModule.types.find(std::make_pair(name, arity));

                if (it != currentModule.types.end()) {
                    COMPILER_ASSERT(!t, "TODO: make sure that a use and a name in the module don't collide");
                    t = it->second;
                }
            }

            if (!t) {
                auto it = builtinTypes.find(name);
                if (it != builtinTypes.end()) {
                    if (arity != 0) {
                        auto err = CompilerError(ErrorCode::UnknownType, signature.start());
                        err.addLabel(std::format("the type {} does not take any type arguments", name), signature.genericStartPos, signature.genericEndPos);
                        return std::unexpected(std::move(err));
                    }

                    t = it->second;
                }
            }

        } else {
            auto path = StringUtils::join(signature.path.parts
                                          | std::ranges::views::transform([](const auto &i) { return i.name; }), "::");
            auto maybeBuiltinType = lookupTypeByPath(path, arity);

            auto firstPart = signature.path.parts[0].name;
            auto usePath = uses.lookup(firstPart);
            if (usePath) {
                auto fullPath = *usePath + StringUtils::join(
                        signature.path.parts | std::ranges::views::drop(1) | std::ranges::views::transform([](const auto &i) { return i.name; }), "::");
                auto maybeType = lookupTypeByPath(fullPath, arity);
                if (maybeType) {
                    t = *maybeType;
                } else {
                    auto err = CompilerError(ErrorCode::UnknownType, signature.start());
                    if (maybeBuiltinType) {
                        err.setNote(std::format("did you mean ::{}", path));
                    }
                    return std::unexpected(std::move(err));
                }
            } else if (maybeBuiltinType) {
                t = *maybeBuiltinType;
            }
        }

        if (!t) {
            return std::unexpected(CompilerError(ErrorCode::UnknownType, signature.start()));
        }
    }

    std::vector<TypeRef> genericArgTypes;
    for (const auto &item: signature.genericArguments) {
        auto res = lookupType(item, generics, currentModuleName, uses);
        if (!res) {
            return res;
        }
        genericArgTypes.emplace_back(std::move(*res));
    }

    if (!t.is<TypeVar>() && !t.is<BuiltinType>()) { // type vars and builtins don't need to worry about accessibility
        auto declModulePath = t.declModulePath();

        if (declModulePath.ends_with("::test") && !currentModuleName.ends_with("::test")) {
            auto err = CompilerError(ErrorCode::InaccessibleType, signature.start());
            err.setNote("types declared in test modules are only accessible from other test modules");
            return std::unexpected(std::move(err));
        }

        if (currentModuleName != declModulePath & currentModuleName != std::string(declModulePath) + "::test" && !t.isPublic()) {
            auto err = CompilerError(ErrorCode::InaccessibleType, signature.start());
            err.addLabel(std::format("{} is defined here", "<TODO: typename>"), t.declStart());
            return std::unexpected(std::move(err));
        }
    }

    if (genericArgTypes.empty()) {
        return t;
    } else {
        return t.concretize(*this, genericArgTypes);
    }
}


bool tuple_typelist_compare(const std::vector<TypeRef> &a, const std::vector<TypeRef> &b) {
    // TODO
    COMPILER_ASSERT(false, "TODO: not implemented");
    return false;
}

bool function_typelist_compare(const std::vector<std::pair<ParameterMode, TypeRef>> &a,
                               const std::vector<std::pair<ParameterMode, TypeRef>> &b) {
    // TODO
    COMPILER_ASSERT(false, "TODO: not implemented");
    return false;
}

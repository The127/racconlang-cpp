#pragma once

#include "predeclare.h"

#include "TypeRef.h"
#include "TypeBase.h"

#include <vector>
#include <memory>
#include <map>

class racc::registry::FunctionType : public TypeBase<FunctionType> {
public:
    std::vector<std::pair<ParameterMode, TypeRef>> parameters;
    std::unique_ptr<TypeRef> returnType;
    bool returnMut;

    FunctionType(std::vector<std::pair<ParameterMode, TypeRef>> parameters, TypeRef returnType, bool returnMut);

    ~FunctionType();

    FunctionType(const FunctionType &) = delete;

    FunctionType &operator=(const FunctionType &) = delete;

    FunctionType(FunctionType &&) noexcept;

    FunctionType &operator=(FunctionType &&) noexcept;

    [[nodiscard]] std::pair<TypeRef, std::shared_ptr<FunctionType>> substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const;
};
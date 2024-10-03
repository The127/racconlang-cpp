#pragma once

#include "predeclare.h"

#include "TypeRef.h"

#include <vector>
#include <memory>
#include <map>

namespace racc::registry {
    enum class ParameterMode : uint8_t {
        Normal,
        Mut,
        Ref,
    };

    class FunctionType {
    public:
        std::vector<std::pair<ParameterMode, TypeRef>> parameters;
        std::unique_ptr<TypeRef> returnType;
        bool returnMut;
        WeakTypeRef type;

        FunctionType(std::vector<std::pair<ParameterMode, TypeRef>> parameters, TypeRef returnType, bool returnMut);

        ~FunctionType();

        FunctionType(const FunctionType &) = delete;

        FunctionType &operator=(const FunctionType &) = delete;

        FunctionType(FunctionType &&) noexcept;

        FunctionType &operator=(FunctionType &&) noexcept;

        [[nodiscard]] TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const;
    };

}

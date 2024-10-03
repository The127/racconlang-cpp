#include "FunctionType.h"

#include "ModuleRegistry.h"
#include "TypeRefImpl.h"

#include <utility>

namespace racc::registry {

    FunctionType::FunctionType(std::vector<std::pair<ParameterMode, TypeRef>> parameters, TypeRef returnType, bool returnMut)
            : parameters(std::move(parameters)),
              returnType(std::make_unique<TypeRef>(std::move(returnType))),
              returnMut(returnMut) {

    }

    TypeRef FunctionType::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const {
        std::vector<std::pair<ParameterMode, TypeRef>> substitutedParameters;
        for (const auto &[mode, param]: parameters) {
            substitutedParameters.emplace_back(mode, param.substituteGenerics(registry, generics));
        }
        auto typeRef = TypeRef::make<FunctionType>(substitutedParameters, returnType->substituteGenerics(registry, generics), returnMut);
        auto e = *typeRef.as<FunctionType>();
        return typeRef;
    }

    FunctionType::~FunctionType() = default;

    FunctionType::FunctionType(FunctionType &&) noexcept = default;

    FunctionType &FunctionType::operator=(FunctionType &&) noexcept = default;

}

#include "FunctionType.h"

#include "ModuleRegistry.h"
#include "TypeRef.h"
#include "Parameter.h"

#include <utility>

namespace racc::registry {

    FunctionType::FunctionType(std::vector<std::pair<ParameterMode, TypeRef>> parameters, TypeRef returnType, bool returnMut)
            : parameters(std::move(parameters)),
              returnType(std::make_unique<TypeRef>(std::move(returnType))),
              returnMut(returnMut) {

    }

    std::pair<TypeRef, std::shared_ptr<FunctionType>> FunctionType::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const {
        std::vector<std::pair<ParameterMode, TypeRef>> substitutedParameters;
        for (const auto &[mode, param]: parameters) {
            substitutedParameters.emplace_back(mode, param.substituteGenerics(registry, generics));
        }
        auto [typeRef, f] = TypeRef::makeFunction(substitutedParameters, returnType->substituteGenerics(registry, generics), returnMut);
        // TODO
        return {typeRef, f};
    }

    FunctionType::~FunctionType() = default;

    FunctionType::FunctionType(FunctionType &&) noexcept = default;

    FunctionType &FunctionType::operator=(FunctionType &&) noexcept = default;

}

//
// Created by zoe on 10.09.24.
//

#include "ConstraintDeclaration.h"
#include "ReturnType.h"
#include "NamelessParameter.h"
#include "utils/NodeUtils.h"

#include "FunctionSignature.h"

namespace racc::ast {

    FunctionSignature::FunctionSignature() = default;

    FunctionSignature::FunctionSignature(FunctionSignature &&) noexcept = default;

    FunctionSignature &FunctionSignature::operator=(FunctionSignature &&) noexcept = default;

    FunctionSignature::~FunctionSignature() = default;

    uint64_t FunctionSignature::start() const {
        return startPos;
    }

    uint64_t FunctionSignature::end() const {
        return endPos;
    }

    std::string FunctionSignature::toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const {
        std::string result = utils::node::nameString(*this, "FunctionSignature", verbose) + "{\n";

        if (!parameterTypes.empty()) {
            result += std::string(indent, ' ') + "parameterTypes: " + utils::node::nodeListString(sources, parameterTypes, indent + 1, verbose) + "\n";
        }

        if (returnType) {
            result += (*returnType).toString(sources, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
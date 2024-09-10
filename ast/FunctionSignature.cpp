//
// Created by zoe on 10.09.24.
//

#include "FunctionSignature.h"

#include "utils/NodeUtils.h"

uint64_t FunctionSignature::start() const {
    return startPos;
}

uint64_t FunctionSignature::end() const {
    return endPos;
}

std::string FunctionSignature::toString(const SourceMap &sources, int indent, bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "FunctionSignature", verbose) + "{\n";

    if(!genericArguments.empty()) {
        result += std::string(indent, ' ') + "genericArguments: " + NodeUtils::nodeListString(sources, genericArguments, indent + 1, verbose) + "\n";
    }
    if(!genericConstraints.empty()) {
        result += std::string(indent, ' ') + "genericConstraints: " + NodeUtils::nodeListString(sources, genericArguments, indent + 1, verbose) + "\n";
    }
    if(!parameterTypes.empty()) {
        result += std::string(indent, ' ') + "parameterTypes: " + NodeUtils::nodeListString(sources, genericArguments, indent + 1, verbose) + "\n";
    }

    if(returnType) {
        result += (*returnType)->toString(sources, indent + 1, verbose) + "\n";
    }

    return std::move(result);
}

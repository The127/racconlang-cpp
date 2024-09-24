//
// Created by zoe on 14.09.24.
//

#include "ImplBlock.h"

#include "utils/NodeUtils.h"
#include "ConstructorDeclaration.h"
#include "ImplSetter.h"
#include "ImplGetter.h"
#include "ImplMethod.h"
#include "DestructureDeclaration.h"
#include "ConstraintDeclaration.h"


ImplBlock::ImplBlock() = default;
ImplBlock::ImplBlock(ImplBlock &&) noexcept = default;
ImplBlock & ImplBlock::operator=(ImplBlock &&) noexcept = default;
ImplBlock::~ImplBlock() = default;

uint64_t ImplBlock::start() const {
    return startPos;
}

uint64_t ImplBlock::end() const {
    return endPos;
}

std::string ImplBlock::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ImplBlock", verbose) + "{\n";

    if (!genericParams.empty()) {
        result += std::string(indent, ' ') + "genericParams: " + NodeUtils::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";
    }

    if(interfaceName) {
        result += std::string(indent, ' ') + "interfaceName: " + interfaceName->toString(sources, indent+1, verbose) + ",\n";
    }
    if(structName) {
        result += std::string(indent, ' ') + "structName: " + structName->toString(sources, indent+1, verbose) + ",\n";
    }
    if(forInterfaceName) {
        result += std::string(indent, ' ') + "forInterfaceName: " + forInterfaceName->toString(sources, indent+1, verbose) + ",\n";
    }

    if (!genericConstraints.empty()) {
        result += std::string(indent, ' ') + "genericConstraints: " + NodeUtils::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";
    }

    if (!constructors.empty()) {
        result += std::string(indent, ' ') + "constructors: " + NodeUtils::nodeListString(sources, constructors, indent + 1, verbose) + "\n";
    }
    if (!destructors.empty()) {
        result += std::string(indent, ' ') + "destructors: " + NodeUtils::nodeListString(sources, destructors, indent + 1, verbose) + "\n";
    }
    if (!methods.empty()) {
        result += std::string(indent, ' ') + "methods: " + NodeUtils::nodeListString(sources, methods, indent + 1, verbose) + "\n";
    }
    if (!setters.empty()) {
        result += std::string(indent, ' ') + "setters: " + NodeUtils::nodeListString(sources, setters, indent + 1, verbose) + "\n";
    }
    if (!getters.empty()) {
        result += std::string(indent, ' ') + "getters: " + NodeUtils::nodeListString(sources, getters, indent + 1, verbose) + "\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return result;
}

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

namespace racc::ast {

    ImplBlock::ImplBlock() = default;

    ImplBlock::ImplBlock(ImplBlock &&) noexcept = default;

    ImplBlock &ImplBlock::operator=(ImplBlock &&) noexcept = default;

    ImplBlock::~ImplBlock() = default;

    uint64_t ImplBlock::start() const {
        return startPos;
    }

    uint64_t ImplBlock::end() const {
        return endPos;
    }

    std::string ImplBlock::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "ImplBlock", verbose) + "{\n";

        if (!genericParams.empty()) {
            result += std::string(indent, ' ') + "genericParams: " + utils::node::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";
        }

        if (interfaceName) {
            result += std::string(indent, ' ') + "interfaceName: " + interfaceName->toString(sources, indent + 1, verbose) + ",\n";
        }
        if (structName) {
            result += std::string(indent, ' ') + "structName: " + structName->toString(sources, indent + 1, verbose) + ",\n";
        }
        if (forInterfaceName) {
            result += std::string(indent, ' ') + "forInterfaceName: " + forInterfaceName->toString(sources, indent + 1, verbose) + ",\n";
        }

        if (!genericConstraints.empty()) {
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";
        }

        if (!constructors.empty()) {
            result += std::string(indent, ' ') + "constructors: " + utils::node::nodeListString(sources, constructors, indent + 1, verbose) + "\n";
        }
        if (!destructors.empty()) {
            result += std::string(indent, ' ') + "destructors: " + utils::node::nodeListString(sources, destructors, indent + 1, verbose) + "\n";
        }
        if (!methods.empty()) {
            result += std::string(indent, ' ') + "methods: " + utils::node::nodeListString(sources, methods, indent + 1, verbose) + "\n";
        }
        if (!setters.empty()) {
            result += std::string(indent, ' ') + "setters: " + utils::node::nodeListString(sources, setters, indent + 1, verbose) + "\n";
        }
        if (!getters.empty()) {
            result += std::string(indent, ' ') + "getters: " + utils::node::nodeListString(sources, getters, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
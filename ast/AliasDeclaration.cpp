//
// Created by zoe on 07.09.24.
//

#include "AliasDeclaration.h"
#include "ConstraintDeclaration.h"
#include "GenericConstraintBase.h"
#include "Signature.h"

#include "utils/NodeUtils.h"
namespace racc::ast {

    AliasDeclaration::AliasDeclaration() = default;

    AliasDeclaration::AliasDeclaration(AliasDeclaration &&) noexcept = default;

    AliasDeclaration &AliasDeclaration::operator=(AliasDeclaration &&) noexcept = default;

    AliasDeclaration::~AliasDeclaration() = default;

    uint64_t AliasDeclaration::start() const {
        return startPos;
    }

    uint64_t AliasDeclaration::end() const {
        return endPos;
    }

    std::string AliasDeclaration::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "AliasDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
        }

        if (!genericParams.empty())
            result += std::string(indent, ' ') + "genericParams: " + utils::node::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";

        if (!genericConstraints.empty())
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";

        if (signature) {
            result += std::string(indent, ' ') + "signature: " + (*signature).toString(sources, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
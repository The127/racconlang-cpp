//
// Created by zoe on 07.09.24.
//

#include "StructDeclaration.h"
#include "PropertyDeclaration.h"
#include "ConstraintDeclaration.h"

#include "utils/NodeUtils.h"
#include "utils/StringUtils.h"

namespace racc::ast {

    StructDeclaration::StructDeclaration() = default;

    StructDeclaration::StructDeclaration(StructDeclaration &&) noexcept = default;

    StructDeclaration &StructDeclaration::operator=(StructDeclaration &&) noexcept = default;

    StructDeclaration::~StructDeclaration() = default;

    uint64_t StructDeclaration::start() const {
        return startPos;
    }

    uint64_t StructDeclaration::end() const {
        return endPos;
    }

    std::string StructDeclaration::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "StructDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

        if (name) {
            result += std::string(indent, ' ') + "name: " + std::string(name->name) + ",\n";
        }

        if (!genericParams.empty())
            result += std::string(indent, ' ') + "genericParams: " + utils::node::nodeListString(sources, genericParams, indent + 1, verbose) + "\n";

        if (!genericConstraints.empty())
            result += std::string(indent, ' ') + "genericConstraints: " + utils::node::nodeListString(sources, genericConstraints, indent + 1, verbose) + "\n";

        if (!propertyDeclarations.empty())
            result +=
                    std::string(indent, ' ') + "propertyDeclarations: " + utils::node::nodeListString(sources, propertyDeclarations, indent + 1, verbose) + "\n";

        result += std::string(indent, ' ') + "destructuresInto: [" + utils::string::join(destructureProperties, ", ") + "],";

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
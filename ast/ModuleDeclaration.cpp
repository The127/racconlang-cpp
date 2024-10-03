//
// Created by zoe on 07.09.24.
//

#include "ModuleDeclaration.h"

#include "EnumDeclaration.h"
#include "FunctionDeclaration.h"
#include "AliasDeclaration.h"
#include "UseMap.h"
#include "InterfaceDeclaration.h"
#include "ModuleVariableDeclaration.h"
#include "StructDeclaration.h"
#include "EnumDeclaration.h"
#include "ConstraintDeclaration.h"
#include "EnumMemberDeclaration.h"
#include "ImplBlock.h"

#include "utils/NodeUtils.h"
#include "utils/StringUtils.h"

namespace racc::ast {

    ModuleDeclaration::ModuleDeclaration() = default;

    ModuleDeclaration::ModuleDeclaration(ModuleDeclaration &&) noexcept = default;

    ModuleDeclaration &ModuleDeclaration::operator=(ModuleDeclaration &&) noexcept = default;

    ModuleDeclaration::~ModuleDeclaration() = default;

    ModuleDeclaration::ModuleDeclaration(Path path)
            : path(std::move(path)) {
    }

    uint64_t ModuleDeclaration::start() const {
        return startPos;
    }

    uint64_t ModuleDeclaration::end() const {
        return endPos;
    }

    std::string ModuleDeclaration::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "ModuleDeclaration", verbose) + "{\n";

        result += std::string(indent, ' ') + "path: " + path.toString(sources, indent, verbose) + ",\n";

        // todo: print use  map
//    if (!uses->uses.empty())
//        result += std::string(indent, ' ') + "uses: " + utils::node::nodeListString(
//            sources, uses->uses, indent + 1, verbose) + "\n";

        if (!moduleVariableDeclarations.empty())
            result += std::string(indent, ' ') + "moduleVariableDeclarations: " + utils::node::nodeListString(
                    sources, moduleVariableDeclarations, indent + 1, verbose) + "\n";

        if (!aliasDeclarations.empty())
            result += std::string(indent, ' ') + "aliasDeclarations: " + utils::node::nodeListString(
                    sources, aliasDeclarations, indent + 1, verbose) + "\n";

        if (!enumDeclarations.empty())
            result += std::string(indent, ' ') + "enumDeclarations: " + utils::node::nodeListString(
                    sources, enumDeclarations, indent + 1, verbose) + "\n";

        if (!interfaceDeclarations.empty())
            result += std::string(indent, ' ') + "interfaceDeclarations: " + utils::node::nodeListString(
                    sources, interfaceDeclarations, indent + 1, verbose) + "\n";

        if (!structDeclarations.empty())
            result += std::string(indent, ' ') + "structDeclarations: " + utils::node::nodeListString(
                    sources, structDeclarations, indent + 1, verbose) + "\n";

        if (!functionDeclarations.empty())
            result += std::string(indent, ' ') + "functionDeclarations: " + utils::node::nodeListString(
                    sources, functionDeclarations, indent + 1, verbose) + "\n";

        if (!implBlocks.empty())
            result += std::string(indent, ' ') + "implBlocks: " + utils::node::nodeListString(
                    sources, implBlocks, indent + 1, verbose) + "\n";

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

    std::string ModuleDeclaration::buildPathString() const {
        return utils::string::join(path.parts, "::");
    }

}
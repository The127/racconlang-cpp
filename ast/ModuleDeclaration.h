//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "AliasDeclaration.h"
#include "EnumDeclaration.h"
#include "FunctionDeclaration.h"
#include "InterfaceDeclaration.h"
#include "ModuleVariableDeclaration.h"
#include "StructDeclaration.h"
#include "UseNode.h"


class ModuleDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    Path path;
    std::vector<UseNode> uses;
    std::vector<EnumDeclaration> enumDeclarations;
    std::vector<InterfaceDeclaration> interfaceDeclarations;
    std::vector<StructDeclaration> structDeclarations;
    std::vector<FunctionDeclaration> functionDeclarations;
    std::vector<AliasDeclaration> aliasDeclarations;
    std::vector<ModuleVariableDeclaration> moduleVariableDeclarations;

    ModuleDeclaration() = default;

    explicit ModuleDeclaration(Path path)
        : path(std::move(path)) {
    }

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

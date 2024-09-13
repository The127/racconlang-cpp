//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Node.h"
#include "Path.h"


class ModuleVariableDeclaration;
class AliasDeclaration;
class FunctionDeclaration;
class StructDeclaration;
class InterfaceDeclaration;
class EnumDeclaration;
class FileUses;

class ModuleDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    std::optional<Path> path;
    std::shared_ptr<FileUses> uses;
    std::vector<EnumDeclaration> enumDeclarations;
    std::vector<InterfaceDeclaration> interfaceDeclarations;
    std::vector<StructDeclaration> structDeclarations;
    std::vector<FunctionDeclaration> functionDeclarations;
    std::vector<AliasDeclaration> aliasDeclarations;
    std::vector<ModuleVariableDeclaration> moduleVariableDeclarations;

    ModuleDeclaration();
    ModuleDeclaration(ModuleDeclaration&&) noexcept;
    ModuleDeclaration& operator=(ModuleDeclaration&&) noexcept;
    ~ModuleDeclaration() override;

    explicit ModuleDeclaration(Path path);

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

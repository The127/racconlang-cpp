//
// Created by zoe on 07.09.24.
//

#pragma once

#include <optional>

#include "Identifier.h"
#include "Node.h"
#include "SignatureBase.h"

class ModuleVariableDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    bool isMut{};
    std::optional<Identifier> name;
    std::optional<std::unique_ptr<SignatureBase>> type;

    ModuleVariableDeclaration();
    ModuleVariableDeclaration(ModuleVariableDeclaration&&) noexcept;
    ModuleVariableDeclaration& operator=(ModuleVariableDeclaration&&) noexcept;
    ~ModuleVariableDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

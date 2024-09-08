//
// Created by zoe on 07.09.24.
//

#pragma once

#include <optional>

#include "Identifier.h"
#include "Node.h"

class ModuleVariableDeclaration final : public Node {
public:
    uint64_t startPos;
    uint64_t endPos;
    bool isPublic;
    std::optional<Identifier> name;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

};

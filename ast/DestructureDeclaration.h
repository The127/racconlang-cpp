//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"


class DestructureDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};

    DestructureDeclaration();
    DestructureDeclaration(DestructureDeclaration&&) noexcept;
    DestructureDeclaration& operator=(DestructureDeclaration&&) noexcept;
    ~DestructureDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

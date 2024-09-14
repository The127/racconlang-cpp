//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"


class ConstructorDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};

    ConstructorDeclaration();
    ConstructorDeclaration(ConstructorDeclaration&&) noexcept;
    ConstructorDeclaration& operator=(ConstructorDeclaration&&) noexcept;
    ~ConstructorDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Identifier.h"
#include "Node.h"


class DestructureDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    std::optional<Identifier> name;

    DestructureDeclaration();
    DestructureDeclaration(const DestructureDeclaration&) = delete;
    DestructureDeclaration& operator=(const DestructureDeclaration&) = delete;
    DestructureDeclaration(DestructureDeclaration&&) noexcept;
    DestructureDeclaration& operator=(DestructureDeclaration&&) noexcept;
    ~DestructureDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"


class ImplBlock final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};

    ImplBlock();
    ImplBlock(ImplBlock&&) noexcept;
    ImplBlock& operator=(ImplBlock&&) noexcept;
    ~ImplBlock() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

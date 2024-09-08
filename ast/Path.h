//
// Created by zoe on 07.09.24.
//


#pragma once
#include <vector>

#include "Identifier.h"
#include "Node.h"


class Path final : public Node {
public:
    std::optional<Token> rooted{};
    std::vector<Identifier> parts{};
    std::optional<Token> trailer{};

    [[nodiscard]] bool isRooted() const;
    [[nodiscard]] bool isTrailing() const;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

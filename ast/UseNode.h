//
// Created by zoe on 07.09.24.
//

#pragma once

#include <vector>

#include "Node.h"
#include "Path.h"


class UseNode final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    Path path;
    std::vector<Identifier> names;

    UseNode();
    UseNode(const UseNode&) = delete;
    UseNode& operator=(const UseNode&) = delete;
    UseNode(UseNode&&) noexcept;
    UseNode& operator=(UseNode&&) noexcept;
    ~UseNode() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

//
// Created by zoe on 07.09.24.
//

#pragma once

#include <vector>

#include "Node.h"
#include "Path.h"


class UseNode final : public Node {
public:
    uint64_t startPos;
    uint64_t endPos;
    Path path;
    std::vector<Identifier> names;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

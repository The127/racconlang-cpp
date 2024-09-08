//
// Created by zoe on 07.09.24.
//

#pragma once

#include <vector>

#include "Node.h"
#include "Path.h"


class UseNode final : Node {
public:
    Path path;
    std::vector<Identifier> names;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

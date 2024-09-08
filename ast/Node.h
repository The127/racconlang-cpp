//
// Created by zoe on 07.09.24.
//

#pragma once

#include <cstdint>


class Node {
public:
    virtual ~Node() = default;

    [[nodiscard]] virtual uint64_t start() const = 0;
    [[nodiscard]] virtual uint64_t end() const = 0;
};

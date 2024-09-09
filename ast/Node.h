//
// Created by zoe on 07.09.24.
//

#pragma once

#include <cstdint>
#include <string>

#include "sourceMap/Source.h"


class Node {
public:
    virtual ~Node() = default;

    [[nodiscard]] virtual uint64_t start() const = 0;
    [[nodiscard]] virtual uint64_t end() const = 0;
    [[nodiscard]] virtual std::string toString(const SourceMap &sources, int indent, bool verbose) const = 0;
};

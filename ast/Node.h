//
// Created by zoe on 07.09.24.
//

#pragma once

#include "predeclare.h"

#include <cstdint>
#include <string>

class racc::ast::Node {
public:
    virtual ~Node();

    [[nodiscard]] virtual uint64_t start() const = 0;

    [[nodiscard]] virtual uint64_t end() const = 0;

    [[nodiscard]] virtual std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const = 0;
};

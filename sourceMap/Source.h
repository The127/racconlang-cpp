//
// Created by zoe on 06.09.24.
//

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Location.h"


class Source {
public:
    const std::string fileName;
    const std::string text;
    const uint64_t offset;
    std::vector<uint32_t> lineBreaks;

    explicit Source(std::string fileName, std::string text, const uint64_t offset)
        : fileName(std::move(fileName)),
          text(std::move(text)),
          offset(offset) {
    }

    void addLineBreak(uint32_t position);

    Location getLocation(uint32_t position);
    [[nodiscard]] std::string_view getText(uint32_t start, uint32_t end) const;
};

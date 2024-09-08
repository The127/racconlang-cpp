//
// Created by zoe on 06.09.24.
//

#pragma once

#include <cstdint>
#include <string>


class Location {
public:
    std::string_view fileName;
    uint32_t line;
    uint32_t column;

    Location(const std::string_view fileName, const uint32_t line, const uint32_t column)
        : fileName(fileName),
          line(line),
          column(column) {
    }

    [[nodiscard]] std::string toString() const;
};

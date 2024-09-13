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

    Location(std::string_view fileName, uint32_t line, uint32_t column);

    [[nodiscard]] std::string toString() const;
};

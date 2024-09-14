//
// Created by zoe on 14.09.24.
//


#pragma once
#include <cstdint>
#include <string>


class SimpleErrorLabel {
public:
    SimpleErrorLabel(std::string text, uint64_t start, uint64_t end);

    [[nodiscard]] uint64_t start() const;
    [[nodiscard]] uint64_t end() const;
    [[nodiscard]] std::string_view text() const;

private:
    std::string _text;
    uint64_t _start;
    uint64_t _end;
};

//
// Created by zoe on 14.09.24.
//


#pragma once

#include <vector>

#include "SimpleErrorLabel.h"

class racc::errors::ErrorLabelGroup {
public:

    ErrorLabelGroup(std::string text, uint64_t start, uint64_t end);

    void addLabel(SimpleErrorLabel label);

    [[nodiscard]] uint64_t start() const;

    [[nodiscard]] uint64_t end() const;

    [[nodiscard]] std::string_view text() const;

private:
    std::string _text;
    uint64_t _start;
    uint64_t _end;
    std::vector<SimpleErrorLabel> labels;
};

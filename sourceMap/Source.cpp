//
// Created by zoe on 06.09.24.
//

#include "Source.h"

void Source::addLineBreak(const uint32_t position) {
    lineBreaks.push_back(position - offset);
}

Location Source::getLocation(uint32_t position) {
    const auto it = std::lower_bound(lineBreaks.begin(), lineBreaks.end(), position);
    const uint32_t lineBreakIndex = std::distance(lineBreaks.begin(), it);

    const auto line = lineBreakIndex + 1;

    uint32_t column = position;
    if (lineBreakIndex > 0) {
        column -= lineBreaks[lineBreakIndex - 1];
    }else {
        position += 1;
    }

    return Location(fileName, line, column);
}

std::string_view Source::getText(const uint32_t start, const uint32_t end) const {
    return std::string_view(text).substr(start, end - start);
}

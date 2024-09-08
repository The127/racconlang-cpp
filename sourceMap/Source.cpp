//
// Created by zoe on 06.09.24.
//

#include "Source.h"
#include "InternalError.h"

void Source::addLineBreak(const uint32_t position) {
    lineBreaks.push_back(position - offset);
}

void Source::addLineComment(uint32_t line, const Token &comment) {
    COMPILER_ASSERT(lineComments.contains(line), std::format("there already is a comment for line {}", line));
    lineComments.emplace(line, comment);
}


Location Source::getLocation(uint32_t position) {
    const auto it = std::lower_bound(lineBreaks.begin(), lineBreaks.end(), position);
    const uint32_t lineBreakIndex = std::distance(lineBreaks.begin(), it);

    const auto line = lineBreakIndex + 1;

    uint32_t column;
    if (lineBreakIndex == 0) {
        column = position + 1;
    } else {
        column = position - lineBreaks[lineBreakIndex - 1];
    }

    return Location(fileName, line, column);
}

std::string_view Source::getText(const uint32_t start, const uint32_t end) const {
    return std::string_view(text).substr(start, end - start);
}
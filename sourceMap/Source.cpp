//
// Created by zoe on 06.09.24.
//

#include "Source.h"
#include "errors/InternalError.h"
#include "errors/CompilerError.h"

namespace racc::sourcemap {

    Source::Source(std::string fileName, std::string text, const uint64_t offset) : fileName(std::move(fileName)),
                                                                                    text(std::move(text)),
                                                                                    offset(offset) {
    }

    Source::Source(Source &&) noexcept = default;

    Source &Source::operator=(Source &&) noexcept = default;

    Source::~Source() = default;

    void Source::addLineBreak(const uint32_t position) {
        lineBreaks.push_back(position - offset);
    }

    void Source::addLineComment(uint32_t line, const lexer::Token &comment) {
        COMPILER_ASSERT(!lineComments.contains(line), std::format("there already is a comment for line {}", line));
        lineComments.emplace(line, comment);
    }

    void Source::addError(errors::CompilerError error) {
        this->errors.emplace_back(std::move(error));
    }


    Location Source::getLocation(const uint32_t position) const {
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

    std::string_view Source::getLine(const uint32_t position) const {
        const auto it = std::lower_bound(lineBreaks.begin(), lineBreaks.end(), position);
        const uint32_t lineBreakIndex = std::distance(lineBreaks.begin(), it);

        const auto line = lineBreakIndex + 1;

        uint32_t start, end;
        if (line == 1) {
            start = 0;
            if (lineBreaks.empty()) {
                end = text.size();
            } else {
                end = lineBreaks[0];
            }
        } else {
            start = lineBreaks[line - 2] + 1;
            if (lineBreaks.size() < line) {
                end = text.size();
            } else {
                end = lineBreaks[line - 1];
            }
        }
        return std::string_view(text).substr(start, end - start);
    }

}

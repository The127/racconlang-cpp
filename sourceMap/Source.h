//
// Created by zoe on 06.09.24.
//

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <map>

#include "CompilerError.h"
#include "Location.h"
#include "lexer/TokenTree.h"


class Source {
public:
    const std::string fileName;
    const std::string text;
    const uint64_t offset;
    std::vector<uint32_t> lineBreaks;
    std::map<uint32_t, Token> lineComments;

    std::optional<TokenTree> tokenTree;
    std::vector<CompilerError> errors;

    explicit Source(std::string fileName, std::string text, const uint64_t offset)
        : fileName(std::move(fileName)),
          text(std::move(text)),
          offset(offset) {
    }

    void addLineBreak(uint32_t position);
    void addLineComment(uint32_t line, const Token& comment);

    Location getLocation(uint32_t position);
    [[nodiscard]] std::string_view getText(uint32_t start, uint32_t end) const;
};

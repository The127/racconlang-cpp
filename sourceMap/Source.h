//
// Created by zoe on 06.09.24.
//

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "lexer/TokenTree.h"


class CompilerError;

class Source {
public:
    std::string fileName;
    std::string text;
    uint64_t offset;
    std::vector<uint32_t> lineBreaks;
    std::map<uint32_t, Token> lineComments;

    std::optional<TokenTree> tokenTree;
    std::vector<CompilerError> errors;

    Source(std::string fileName, std::string text, uint64_t offset);

    Source(const Source&) = delete;
    Source& operator=(const Source&) = delete;
    Source(Source&&) noexcept;
    Source& operator=(Source&&) noexcept;
    ~Source();

    void addLineBreak(uint32_t position);
    void addLineComment(uint32_t line, const Token& comment);
    void addError(CompilerError error);

    [[nodiscard]] Location getLocation(uint32_t position) const;
    [[nodiscard]] std::string_view getText(uint32_t start, uint32_t end) const;
};

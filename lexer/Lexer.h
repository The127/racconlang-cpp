//
// Created by zoe on 06.09.24.
//

#pragma once

#include <utility>

#include "TokenTree.h"
#include "../sourceMap/Source.h"


class Lexer {
public:
    Source& source;

    explicit Lexer(Source& source)
        : source(source) {
    }

    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;
    Lexer(Lexer&&) = default;

    TokenTree tokenize();

private:
    uint32_t position = 0;
    std::optional<TokenResult> peeked;

    TokenResult nextToken();

    void consumeWhitespace();

    const TokenResult& peekToken();

    [[nodiscard]] TokenResult colonRule() const;
    [[nodiscard]] TokenResult dashRule() const;
    [[nodiscard]] TokenResult equalsRule() const;
    [[nodiscard]] TokenResult slashRule() const;
    [[nodiscard]] TokenResult identifierRule() const;
    TokenResult quotedStringRule();
};

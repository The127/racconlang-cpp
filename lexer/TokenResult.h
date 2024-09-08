//
// Created by zoe on 07.09.24.
//


#pragma once
#include <expected>

#include "LexerErr.h"


class TokenResult {
public:
    std::expected<Token, LexerErr> value;

    // ReSharper disable CppNonExplicitConvertingConstructor
    TokenResult(const Token &token)
        : value(token) {
    }

    // ReSharper disable CppNonExplicitConvertingConstructor
    TokenResult(const LexerErr& err)
        : value(std::unexpected(err)) {
    }

    [[nodiscard]] uint64_t getStart() const;

    [[nodiscard]] uint64_t getEnd() const;

    [[nodiscard]] bool isError() const;
    [[nodiscard]] bool isToken() const;
    [[nodiscard]] bool isToken(TokenType) const;

    [[nodiscard]] Token &get();

    [[nodiscard]] const Token &get() const;

    [[nodiscard]] const LexerErr &getError() const;

    [[nodiscard]] Location getPosition(const SourceMap &sources) const;
};

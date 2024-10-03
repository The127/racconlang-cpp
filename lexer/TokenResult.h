//
// Created by zoe on 07.09.24.
//


#pragma once
#include <expected>

#include "LexerErr.h"
#include "sourceMap/Location.h"

namespace racc::lexer {

    class TokenResult {
    public:
        std::expected<Token, LexerErr> value;

        explicit(false) TokenResult(const Token &token);

        explicit(false) TokenResult(LexerErr err);

        TokenResult(const TokenResult &) = delete;

        TokenResult &operator=(const TokenResult &) = delete;

        TokenResult(TokenResult &&) noexcept;

        TokenResult &operator=(TokenResult &&) noexcept;

        ~TokenResult();

        [[nodiscard]] uint64_t getStart() const;

        [[nodiscard]] uint64_t getEnd() const;

        [[nodiscard]] bool isError() const;

        [[nodiscard]] bool isToken() const;

        [[nodiscard]] bool isToken(TokenType) const;

        [[nodiscard]] Token &get();

        [[nodiscard]] const Token &get() const;

        [[nodiscard]] const LexerErr &getError() const;

        [[nodiscard]] const Token &getOrErrorToken() const;

        [[nodiscard]] sourcemap::Location getLocation(const sourcemap::SourceMap &sources) const;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources) const;
    };

}

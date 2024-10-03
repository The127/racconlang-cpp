//
// Created by zoe on 06.09.24.
//

#pragma once

#include "TokenTree.h"
#include "sourceMap/Source.h"

namespace racc::lexer {

    class Lexer {
    public:
        std::shared_ptr<sourcemap::Source> source;

        explicit Lexer(std::shared_ptr<sourcemap::Source> source);

        Lexer(const Lexer &) = delete;

        Lexer &operator=(const Lexer &) = delete;

        Lexer(Lexer &&) noexcept;

        Lexer &operator=(Lexer &&) noexcept;

        ~Lexer();

        void tokenize();

    private:
        uint32_t position = 0;
        std::optional<TokenResult> peeked;

        TokenResult nextToken();

        void consumeWhitespace();

        const TokenResult &peekToken();

        [[nodiscard]] TokenResult colonRule() const;

        [[nodiscard]] TokenResult dashRule() const;

        [[nodiscard]] TokenResult equalsRule() const;

        [[nodiscard]] TokenResult slashRule() const;

        [[nodiscard]] TokenResult identifierRule() const;

        TokenResult quotedStringRule();
    };

}

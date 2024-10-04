//
// Created by zoe on 06.09.24.
//

#pragma once

#include "predeclare.h"

#include "TokenType.h"

#include <cstdint>
#include <string>

class racc::lexer::Token {
public:
    TokenType type;
    uint64_t start;
    uint64_t end;

    Token(TokenType type, u_int64_t start, u_int64_t end);

    Token(const Token &);

    Token &operator=(const Token &);

    Token(Token &&) noexcept;

    Token &operator=(Token &&) noexcept;

    [[nodiscard]] bool isComment() const;

    [[nodiscard]] bool isAllowedInAngleBrackets() const;

    [[nodiscard]] bool isOpening() const;

    [[nodiscard]] bool isClosing() const;

    [[nodiscard]] bool isDeclaratorKeyword() const;

    [[nodiscard]] bool isTopLevelStarter() const;

    [[nodiscard]] bool isModifier() const;

    [[nodiscard]] bool isConstraintBreakout() const;

    [[nodiscard]] bool isSignatureStarter() const;

    [[nodiscard]] bool isTypeSignatureStarter() const;

    [[nodiscard]] bool isPathStarter() const;

    [[nodiscard]] TokenType expectedClosing() const;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources) const;
};

//
// Created by zoe on 07.09.24.
//

#pragma once

#include <utility>
#include <vector>
#include <map>

#include "Token.h"
#include "../errors/InternalError.h"

enum class LexerErrReason {
    UnclosedTokenTree,
    UnexpectedInput,
    UnexpectedEndOfInput,
    InvalidIdentifier,
};

class LexerErr {
public:
    LexerErrReason reason;
    Token got;
    std::vector<std::string> expected;

    LexerErr(LexerErrReason reason, const Token &got, std::vector<std::string> expected);

    LexerErr(LexerErr&&) noexcept;
    LexerErr& operator=(LexerErr&&) noexcept;
    ~LexerErr();

    static LexerErr UnclosedTokenTree(const Token &got, std::string expected);

    static LexerErr UnclosedTokenTree(u_int64_t start, u_int64_t end, std::string expected);

    static LexerErr UnexpectedInput(const Token &got, std::vector<std::string> expected);

    static LexerErr UnexpectedInput(u_int64_t start, u_int64_t end, std::vector<std::string> expected);

    static LexerErr UnexpectedInput(const Token &got, std::string expected);

    static LexerErr UnexpectedInput(u_int64_t start, u_int64_t end, std::string expected);

    static LexerErr UnexpectedInput(const Token &got);

    static LexerErr UnexpectedInput(u_int64_t start, u_int64_t end);

    static LexerErr UnexpectedEndOfInput(const Token &got, std::vector<std::string> expected);

    static LexerErr UnexpectedEndOfInput(u_int64_t pos, std::vector<std::string> expected);

    static LexerErr UnexpectedEndOfInput(const Token &got, std::string expected);

    static LexerErr UnexpectedEndOfInput(u_int64_t pos, std::string expected);

    static LexerErr UnexpectedEndOfInput(const Token &got);

    static LexerErr UnexpectedEndOfInput(u_int64_t pos);

    static LexerErr InvalidIdentifier(const Token &got);

    static LexerErr InvalidIdentifier(u_int64_t start, u_int64_t end);

    [[nodiscard]] std::string toString(const SourceMap &sources) const;

private:
    [[nodiscard]] std::string expectedString() const;
};

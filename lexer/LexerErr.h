//
// Created by zoe on 07.09.24.
//

#pragma once

#include "predeclare.h"

#include <utility>
#include <vector>
#include <map>

#include "Token.h"
#include "errors/InternalError.h"

// TODO: move to racc::errors?

enum class racc::lexer::LexerErrReason {
    UnclosedTokenTree,
    UnexpectedInput,
    UnexpectedEndOfInput,
    InvalidIdentifier,
};

class racc::lexer::LexerErr {
public:
    LexerErrReason reason;
    lexer::Token got;
    std::vector<std::string> expected;

    LexerErr(LexerErrReason reason, const lexer::Token &got, std::vector<std::string> expected);

    LexerErr(const LexerErr &) = delete;

    LexerErr &operator=(const LexerErr &) = delete;

    LexerErr(LexerErr &&) noexcept;

    LexerErr &operator=(LexerErr &&) noexcept;

    ~LexerErr();

    static LexerErr UnclosedTokenTree(const lexer::Token &got, std::string expected);

    static LexerErr UnclosedTokenTree(u_int64_t start, u_int64_t end, std::string expected);

    static LexerErr UnexpectedInput(const lexer::Token &got, std::vector<std::string> expected);

    static LexerErr UnexpectedInput(u_int64_t start, u_int64_t end, std::vector<std::string> expected);

    static LexerErr UnexpectedInput(const lexer::Token &got, std::string expected);

    static LexerErr UnexpectedInput(u_int64_t start, u_int64_t end, std::string expected);

    static LexerErr UnexpectedInput(const lexer::Token &got);

    static LexerErr UnexpectedInput(u_int64_t start, u_int64_t end);

    static LexerErr UnexpectedEndOfInput(const lexer::Token &got, std::vector<std::string> expected);

    static LexerErr UnexpectedEndOfInput(u_int64_t pos, std::vector<std::string> expected);

    static LexerErr UnexpectedEndOfInput(const lexer::Token &got, std::string expected);

    static LexerErr UnexpectedEndOfInput(u_int64_t pos, std::string expected);

    static LexerErr UnexpectedEndOfInput(const lexer::Token &got);

    static LexerErr UnexpectedEndOfInput(u_int64_t pos);

    static LexerErr InvalidIdentifier(const lexer::Token &got);

    static LexerErr InvalidIdentifier(u_int64_t start, u_int64_t end);

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources) const;

private:
    [[nodiscard]] std::string expectedString() const;
};

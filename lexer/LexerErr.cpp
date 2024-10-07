//
// Created by zoe on 07.09.24.
//

#include "LexerErr.h"

#include "sourceMap/Location.h"
#include "sourceMap/SourceMap.h"

namespace racc::lexer {

    LexerErr::LexerErr(LexerErrReason reason, const lexer::Token &got, std::vector<std::string> expected) : reason(reason), got(got), expected(std::move(expected)) {
    }

    LexerErr::LexerErr(LexerErr &&) noexcept = default;

    LexerErr &LexerErr::operator=(LexerErr &&) noexcept = default;

    LexerErr::~LexerErr() = default;

    LexerErr LexerErr::UnclosedTokenTree(const lexer::Token &got, std::string expected) {
        return LexerErr(LexerErrReason::UnclosedTokenTree, got, {std::move(expected)});
    }

    LexerErr LexerErr::UnclosedTokenTree(const u_int64_t start, const u_int64_t end, std::string expected) {
        return UnclosedTokenTree(Token(TokenType::Error, start, end), {std::move(expected)});
    }

    LexerErr LexerErr::UnexpectedInput(const lexer::Token &got, std::vector<std::string> expected) {
        return LexerErr(LexerErrReason::UnexpectedInput, got, std::move(expected));
    }

    LexerErr LexerErr::UnexpectedInput(const u_int64_t start, const u_int64_t end, std::vector<std::string> expected) {
        return UnexpectedInput(Token(TokenType::Error, start, end), std::move(expected));
    }

    LexerErr LexerErr::UnexpectedInput(const lexer::Token &got, std::string expected) {
        return UnexpectedInput(got, std::vector<std::string>{std::move(expected)});
    }

    LexerErr LexerErr::UnexpectedInput(const u_int64_t start, const u_int64_t end, std::string expected) {
        return UnexpectedInput(start, end, std::vector<std::string>{std::move(expected)});
    }

    LexerErr LexerErr::UnexpectedInput(const lexer::Token &got) {
        return UnexpectedInput(got, std::vector<std::string>{});
    }

    LexerErr LexerErr::UnexpectedEndOfInput(const lexer::Token &got, std::vector<std::string> expected) {
        return LexerErr(LexerErrReason::UnexpectedEndOfInput, got, std::move(expected));
    }

    LexerErr LexerErr::UnexpectedEndOfInput(const lexer::Token &got, std::string expected) {
        return UnexpectedEndOfInput(got, std::vector<std::string>{std::move(expected)});
    }

    LexerErr LexerErr::UnexpectedEndOfInput(const lexer::Token &got) {
        return UnexpectedEndOfInput(got, std::vector<std::string>{});
    }

    LexerErr LexerErr::InvalidIdentifier(const lexer::Token &got) {
        return LexerErr(LexerErrReason::InvalidIdentifier, got, {});
    }

    LexerErr LexerErr::InvalidIdentifier(const u_int64_t start, const u_int64_t end) {
        return InvalidIdentifier(Token(TokenType::Error, start, end));
    }

    std::string LexerErr::toString(const sourcemap::SourceMap &sources) const {
        auto loc = sources.getLocation(got.start);
        switch (reason) {
            case LexerErrReason::UnclosedTokenTree:
                COMPILER_ASSERT(!expected.empty(), "expected is empty for UnclosedTokenTree error");
                return std::format("unclosed lexer::Token tree at {}, expected {}", loc, expectedString());

            case LexerErrReason::UnexpectedInput:
                if (expected.empty()) {
                    return std::format("unexpected input `{}` at {}", sources.getText(got), loc);
                } else {
                    return std::format("unexpected input `{}` at {}, expected {}", sources.getText(got), loc, expectedString());
                }

            case LexerErrReason::UnexpectedEndOfInput:
                if (expected.empty()) {
                    return std::format("unexpected end of input at {}", loc);
                } else {
                    return std::format("unexpected end of input at {}, expected {}", loc, expectedString());
                }

            case LexerErrReason::InvalidIdentifier:
                COMPILER_ASSERT(expected.empty(), "expected is not empty for InvalidIdentifier error");
                return std::format("invalid identifier `{}` at {}", sources.getText(got), loc);

            default:
                COMPILER_ASSERT(false, "unhandled lexer error");
                return "";
        }
    }

    std::string LexerErr::expectedString() const {
        COMPILER_ASSERT(!expected.empty(), "expected is empty in call to expectedString()");

        if (expected.size() == 1) {
            return expected[0];
        }
        size_t size = expected.size() - 1;
        for (size_t i = 0; i < expected.size() - 1; i++) {
            size += expected[i].size();
        }

        std::string commaList;
        commaList.reserve(size);
        for (size_t i = 0; i < expected.size() - 2; i++) {
            commaList += expected[i];
        }
        commaList += expected[expected.size() - 2];

        return std::format("{} or {}", commaList, expected.back());
    }

    LexerErr LexerErr::UnexpectedEndOfInput(const u_int64_t pos) {
        return UnexpectedEndOfInput(pos, std::vector<std::string>{});
    }

    LexerErr LexerErr::UnexpectedEndOfInput(const u_int64_t pos, std::string expected) {
        return UnexpectedEndOfInput(pos, std::vector<std::string>{std::move(expected)});
    }

    LexerErr LexerErr::UnexpectedEndOfInput(const u_int64_t pos, std::vector<std::string> expected) {
        return UnexpectedEndOfInput(Token(TokenType::Eof, pos, pos), std::move(expected));
    }

    LexerErr LexerErr::UnexpectedInput(const u_int64_t start, const u_int64_t end) {
        return UnexpectedInput(Token(TokenType::Error, start, end));
    }

}

//
// Created by zoe on 07.09.24.
//


#pragma once

#include "predeclare.h"

#include "ErrorCode.h"

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#ifndef NDEBUG

#include <stacktrace>

#endif

class racc::errors::CompilerError {
public:
    ErrorCode code;
    uint64_t position;
    std::vector<ErrorLabel> labels;
    std::optional<std::string> note;
#ifndef NDEBUG
    std::stacktrace stacktrace;
#endif


    CompilerError(ErrorCode code, uint64_t position);

    CompilerError(ErrorCode code, const lexer::Token &token);

    CompilerError(const CompilerError &) = delete;

    CompilerError &operator=(const CompilerError &) = delete;

    CompilerError(CompilerError &&) noexcept;

    CompilerError &operator=(CompilerError &&) noexcept;

    ~CompilerError();

    void addLabel(const ErrorLabel &label);

    void addLabel(const std::string &text, uint64_t start, uint64_t end);

    void addLabel(const std::string &text, uint64_t pos);

    void addLabel(const std::string &text, const lexer::Token &startToken, const lexer::Token &endToken);

    void addLabel(const std::string &text, const lexer::Token &token);

    void addLabel(const std::string &text, const lexer::TokenResult &result);

    void addLabel(const std::string &text, const lexer::TokenResult &startResult, const lexer::TokenResult &endResult);

    void addLabel(const std::string &text, const lexer::TokenTree &tree);

    void addLabel(const std::string &text, const lexer::TokenTreeNode &node);

    void setNote(const std::string &note);
};

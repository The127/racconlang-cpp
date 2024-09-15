//
// Created by zoe on 07.09.24.
//


#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ErrorCode.h"
#include "ErrorLabel.h"
#include "../lexer/Token.h"
#include "../lexer/TokenResult.h"
#include "../lexer/TokenTree.h"

class CompilerError {
public:
    ErrorCode code;
    uint64_t position;
    std::vector<ErrorLabel> labels;
    std::optional<std::string> note;
#ifndef NDEBUG
    std::stacktrace stacktrace;
#endif


    CompilerError(ErrorCode code, uint64_t position);
    CompilerError(ErrorCode code, const Token &token);

    CompilerError(CompilerError&&) noexcept;
    CompilerError& operator=(CompilerError&&) noexcept;
    ~CompilerError();

    void addLabel(const ErrorLabel &label);

    void addLabel(const std::string &text, uint64_t start, uint64_t end);

    void addLabel(const std::string &text, const Token &startToken, const Token &endToken);

    void addLabel(const std::string &text, const Token &token);

    void addLabel(const std::string &text, const TokenResult &result);

    void addLabel(const std::string &text, const TokenResult &startResult, const TokenResult &endResult);

    void addLabel(const std::string &text, const TokenTree &tree);

    void addLabel(const std::string &text, const TokenTreeNode &node);

    void setNote(const std::string& note);
};

//
// Created by zoe on 07.09.24.
//


#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lexer/Token.h"


enum ErrorCode {
    PathHasTrailingSeparator,
    UseIsMissingPath,
    MissingSemicolon,
};

class ErrorLabel {
public:
    std::string text;
    uint64_t start;
    uint64_t end;

    ErrorLabel(std::string text, const uint64_t start, const uint64_t end)
        : text(std::move(text)), start(start), end(end) {
    }

    ErrorLabel(std::string text, const Token &startToken, const Token &endToken)
        : text(std::move(text)), start(startToken.start), end(endToken.end) {
    }

    ErrorLabel(std::string text, const Token &token)
        : text(std::move(text)), start(token.start), end(token.end) {
    }
};

class CompilerError {
public:
    ErrorCode code;
    uint64_t position;
    std::vector<ErrorLabel> labels;
    std::optional<std::string> note;


    CompilerError(const ErrorCode code, const uint64_t position)
        : code(code), position(position) {
    }

    CompilerError(const ErrorCode code, const Token &token)
        : code(code), position(token.start) {
    }

    void addLabel(const ErrorLabel &label) {
        labels.push_back(label);
    }

    void addLabel(const std::string &text, const uint64_t start, const uint64_t end) {
        labels.emplace_back(text, start, end);
    }

    void addLabel(const std::string &text, const Token &startToken, const Token &endToken) {
        labels.emplace_back(text, startToken, endToken);
    }

    void addLabel(const std::string &text, const Token &token) {
        labels.emplace_back(text, token);
    }

    void setNote(const std::string& note) {
        this->note = note;
    }
};

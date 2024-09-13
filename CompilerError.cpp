//
// Created by zoe on 07.09.24.
//

#include "CompilerError.h"

ErrorLabel::ErrorLabel(std::string text, const uint64_t start, const uint64_t end): text(std::move(text)), start(start), end(end) {
}

CompilerError::CompilerError(const ErrorCode code, const uint64_t position): code(code), position(position) {
}

CompilerError::CompilerError(const ErrorCode code, const Token &token): code(code), position(token.start) {
}

CompilerError::CompilerError(CompilerError &&) noexcept = default;
CompilerError & CompilerError::operator=(CompilerError &&) noexcept = default;

CompilerError::~CompilerError() = default;

void CompilerError::addLabel(const ErrorLabel &label) {
    labels.push_back(label);
}

void CompilerError::addLabel(const std::string &text, const uint64_t start, const uint64_t end) {
    labels.emplace_back(text, start, end);
}

void CompilerError::addLabel(const std::string &text, const Token &startToken, const Token &endToken) {
    labels.emplace_back(text, startToken.start, endToken.end);
}

void CompilerError::addLabel(const std::string &text, const Token &token) {
    addLabel(text, token, token);
}

void CompilerError::addLabel(const std::string &text, const TokenResult &result) {
    addLabel(text, result.getOrErrorToken());
}

void CompilerError::addLabel(const std::string &text, const TokenResult &startResult, const TokenResult &endResult) {
    addLabel(text, startResult.getOrErrorToken(), endResult.getOrErrorToken());
}

void CompilerError::addLabel(const std::string &text, const TokenTree &tree) {
    addLabel(text, tree.left, tree.right.getOrErrorToken());
}

void CompilerError::addLabel(const std::string &text, const TokenTreeNode &node) {
    addLabel(text, node.getStart(), node.getEnd());
}

void CompilerError::setNote(const std::string &note) {
    this->note = note;
}

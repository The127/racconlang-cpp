//
// Created by zoe on 07.09.24.
//

#include "CompilerError.h"
#include "lexer/TokenTreeNode.h"
#include "SimpleErrorLabel.h"
#include "ErrorLabel.h"

namespace racc::errors {

    CompilerError::CompilerError(const ErrorCode code, const uint64_t position)
            : code(code),
              position(position)
#ifndef NDEBUG
#ifdef __cpp_lib_stacktrace
    , stacktrace(std::stacktrace::current())
#endif
#endif
    {
    }

    CompilerError::CompilerError(const ErrorCode code, const lexer::Token &token)
            : code(code),
              position(token.start) {
    }

    CompilerError::CompilerError(CompilerError &&) noexcept = default;

    CompilerError &CompilerError::operator=(CompilerError &&) noexcept = default;

    CompilerError::~CompilerError() = default;

    void CompilerError::addLabel(const ErrorLabel &label) {
        labels.push_back(label);
    }

    void CompilerError::addLabel(const std::string &text, const uint64_t start, const uint64_t end) {
        addLabel(SimpleErrorLabel(text, start, end));
    }

    void CompilerError::addLabel(const std::string &text, const uint64_t pos) {
        addLabel(SimpleErrorLabel(text, pos, pos));
    }

    void CompilerError::addLabel(const std::string &text, const lexer::Token &startToken, const lexer::Token &endToken) {
        addLabel(SimpleErrorLabel(text, startToken.start, endToken.end));
    }

    void CompilerError::addLabel(const std::string &text, const lexer::Token &token) {
        addLabel(text, token, token);
    }

    void CompilerError::addLabel(const std::string &text, const lexer::TokenResult &result) {
        addLabel(text, result.getOrErrorToken());
    }

    void CompilerError::addLabel(const std::string &text, const lexer::TokenResult &startResult, const lexer::TokenResult &endResult) {
        addLabel(text, startResult.getOrErrorToken(), endResult.getOrErrorToken());
    }

    void CompilerError::addLabel(const std::string &text, const lexer::TokenTree &tree) {
        addLabel(text, tree.left, tree.right.getOrErrorToken());
    }

    void CompilerError::addLabel(const std::string &text, const lexer::TokenTreeNode &node) {
        addLabel(text, node.getStart(), node.getEnd());
    }

    void CompilerError::setNote(const std::string &note) {
        this->note = note;
    }

}
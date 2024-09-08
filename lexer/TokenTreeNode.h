//
// Created by zoe on 07.09.24.
//


#pragma once
#include <variant>

#include "TokenResult.h"
#include "TokenTree.h"

class TokenTreeNode {
public:
    std::variant<TokenTree, TokenResult> value;
    std::vector<Token> precedingComments;

    explicit(false) TokenTreeNode(const TokenTree& tree, std::vector<Token> comments)
        : value(tree), precedingComments(std::move(comments)) {
    }

    explicit(false) TokenTreeNode(const TokenResult& result, std::vector<Token> comments)
        : value(result), precedingComments(std::move(comments)) {
    }

    [[nodiscard]] bool isTokenTree() const;
    [[nodiscard]] bool isTokenTree(TokenType type) const;
    [[nodiscard]] bool isTokenResult() const;
    [[nodiscard]] bool isToken() const;
    [[nodiscard]] bool isToken(TokenType type) const;
    [[nodiscard]] bool isError() const;

    [[nodiscard]] bool isConstraintBreakout() const;

    [[nodiscard]] Location getPosition(const SourceMap &sources) const;
    [[nodiscard]] uint64_t getStart() const;
    [[nodiscard]] uint64_t getEnd() const;

    [[nodiscard]] const TokenTree &getTokenTree() const;
    [[nodiscard]] const TokenResult &getTokenResult() const;
    [[nodiscard]] const Token &getToken() const;
    [[nodiscard]] const LexerErr &getError() const;

    [[nodiscard]] std::string toString(const SourceMap & sources, uint32_t indent) const;
};

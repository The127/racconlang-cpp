//
// Created by zoe on 07.09.24.
//


#pragma once

#include <variant>

#include "TokenResult.h"
#include "TokenTree.h"

class racc::lexer::TokenTreeNode {
public:
    std::variant<TokenTree, TokenResult> value;
    std::vector<Token> precedingComments;

    explicit(false) TokenTreeNode(TokenTree tree, std::vector<Token> comments);

    explicit(false) TokenTreeNode(TokenResult result, std::vector<Token> comments);

    TokenTreeNode(const TokenTreeNode &) = delete;

    TokenTreeNode &operator=(const TokenTreeNode &) = delete;

    TokenTreeNode(TokenTreeNode &&) noexcept;

    TokenTreeNode &operator=(TokenTreeNode &&) noexcept;

    ~TokenTreeNode();

    [[nodiscard]] bool isTokenTree() const;

    [[nodiscard]] bool isTokenTree(TokenType type) const;

    [[nodiscard]] bool isTokenResult() const;

    [[nodiscard]] bool isToken() const;

    [[nodiscard]] bool isToken(TokenType type) const;

    [[nodiscard]] bool isError() const;

    [[nodiscard]] bool isConstraintBreakout() const;

    [[nodiscard]] bool isTypeSignatureStarter() const;

    [[nodiscard]] bool isSignatureStarter() const;

    [[nodiscard]] bool isPathStarter() const;

    [[nodiscard]] bool isTopLevelStarter() const;

    [[nodiscard]] bool isModifier() const;

    [[nodiscard]] bool isDeclaratorKeyword() const;

    [[nodiscard]] sourcemap::Location getPosition(const sourcemap::SourceMap &sources) const;

    [[nodiscard]] uint64_t getStart() const;

    [[nodiscard]] uint64_t getEnd() const;

    [[nodiscard]] const TokenTree &getTokenTree() const;

    [[nodiscard]] const TokenResult &getTokenResult() const;

    [[nodiscard]] const Token &getToken() const;

    [[nodiscard]] const LexerErr &getError() const;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, uint32_t indent) const;

    [[nodiscard]] std::string debugString() const;
};

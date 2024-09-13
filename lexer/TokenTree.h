//
// Created by zoe on 06.09.24.
//

#pragma once

#include <vector>

#include "Token.h"
#include "TokenResult.h"

class TokenTreeNode;

class TokenTree {
public:
    Token left;
    std::vector<TokenTreeNode> tokens;
    TokenResult right;

    explicit TokenTree(const Token &left);
    TokenTree(TokenTree&&) noexcept;
    TokenTree& operator=(TokenTree&&) noexcept;
    ~TokenTree();

    [[nodiscard]] std::string toString(const SourceMap &sources, uint32_t indent = 0) const;
};

#include "TokenTreeNode.h"
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
    std::vector<TokenTreeNode> tokens{};
    TokenResult right;

    explicit TokenTree(const Token &left)
        : left(left),
          right(Token(TokenType::Error, 0, 0)) {
    }

    [[nodiscard]] std::string toString(const SourceMap &sources, uint32_t indent) const;
    [[nodiscard]] std::string toString(const SourceMap &sources) const;
};

#include "TokenTreeNode.h"
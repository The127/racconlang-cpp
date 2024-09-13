//
// Created by zoe on 07.09.24.
//

#include "TokenTreeNode.h"
#include "sourceMap/SourceMap.h"

TokenTreeNode::TokenTreeNode(TokenTree tree, std::vector<Token> comments): value(std::move(tree)), precedingComments(std::move(comments)) {
}

TokenTreeNode::TokenTreeNode(TokenResult result, std::vector<Token> comments): value(std::move(result)), precedingComments(std::move(comments)) {
}

TokenTreeNode::TokenTreeNode(TokenTreeNode &&) noexcept = default;
TokenTreeNode & TokenTreeNode::operator=(TokenTreeNode &&) noexcept = default;
TokenTreeNode::~TokenTreeNode() = default;

bool TokenTreeNode::isTokenTree() const {
    return std::holds_alternative<TokenTree>(value);
}

bool TokenTreeNode::isTokenTree(TokenType type) const {
    return isTokenTree() && getTokenTree().left.type == type;
}

bool TokenTreeNode::isTokenResult() const {
    return std::holds_alternative<TokenResult>(value);
}

bool TokenTreeNode::isToken() const {
    return isTokenResult() && getTokenResult().isToken();
}

bool TokenTreeNode::isToken(TokenType type) const {
    return isTokenResult() && getTokenResult().isToken(type);
}

bool TokenTreeNode::isError() const {
    return isTokenResult() && getTokenResult().isError();
}

Location TokenTreeNode::getPosition(const SourceMap &sources) const {
    return sources.getLocation(getStart());
}

uint64_t TokenTreeNode::getStart() const {
    if (isTokenTree())
        return getTokenTree().left.start;
    return getTokenResult().getStart();
}

uint64_t TokenTreeNode::getEnd() const {
    if (isTokenTree())
        return getTokenTree().right.getEnd();
    return getTokenResult().getEnd();
}

const TokenTree &TokenTreeNode::getTokenTree() const {
    return std::get<TokenTree>(value);
}

const TokenResult &TokenTreeNode::getTokenResult() const {
    return std::get<TokenResult>(value);
}

const Token &TokenTreeNode::getToken() const {
    return getTokenResult().get();
}

const LexerErr &TokenTreeNode::getError() const {
    return getTokenResult().getError();
}

std::string TokenTreeNode::toString(const SourceMap &sources, uint32_t indent) const {
    if (isTokenTree()) {
        return getTokenTree().toString(sources, indent);
    }
    return std::string(indent, ' ') + getTokenResult().toString(sources) + "\n";
}

bool TokenTreeNode::isConstraintBreakout() const {
    return isTokenTree(TokenType::OpenCurly) || (isToken() && getToken().isConstraintBreakout());
}

bool TokenTreeNode::isSignatureStarter() const {
    return isTokenTree(TokenType::OpenParen) || isToken() && getToken().isSignatureStarter();
}

bool TokenTreeNode::isTopLevelStarter() const {
    return isToken() && getToken().isTopLevelStarter();
}

bool TokenTreeNode::isModifier() const {
    return isToken() && getToken().isTopLevelStarter();
}

bool TokenTreeNode::isDeclaratorKeyword() const {
    return isToken() && getToken().isTopLevelStarter();
}

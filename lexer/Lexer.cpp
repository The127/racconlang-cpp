//
// Created by zoe on 06.09.24.
//

#include "Lexer.h"

#include "errors/InternalError.h"
#include "TokenTreeNode.h"

#include <utility>
#include <algorithm>


namespace racc::lexer {

    Lexer::Lexer(std::shared_ptr<sourcemap::Source> source) : source(std::move(source)) {
    }

    Lexer::Lexer(Lexer &&) noexcept = default;

    Lexer &Lexer::operator=(Lexer &&) noexcept = default;

    Lexer::~Lexer() = default;

    void Lexer::tokenize() {
        COMPILER_ASSERT(!source->tokenTree, "Token tree was already set: " + source->fileName);
        std::vector<TokenTree> stack{};
        std::vector<std::vector<Token>> commentStack{};

        stack.emplace_back(Token(TokenType::Bof, source->offset, source->offset));
        commentStack.emplace_back();

        std::vector<Token> pending_comments;

        while (true) {
            auto tokenResult = nextToken();

            if (tokenResult.isToken(TokenType::MultiLineComment)) {
                pending_comments.push_back(tokenResult.get());
                continue;
            }
            if (tokenResult.isToken(TokenType::LineComment)) {
                auto loc = source->getLocation(tokenResult.getStart() - source->offset);
                source->addLineComment(loc.line, tokenResult.get());
                continue;
            }

            if (tokenResult.isError()) {
                stack.back().tokens.emplace_back(std::move(tokenResult), std::move(pending_comments));
                pending_comments = {};
                continue;
            }

            auto &token = tokenResult.get();

            if (token.isOpening()) {
                stack.emplace_back(token);
                commentStack.emplace_back(std::move(pending_comments));
                pending_comments = {};
                continue;
            }

            if (token.type == stack.back().left.expectedClosing()) {
                stack.back().right = std::move(tokenResult);
                auto temp = std::move(stack.back());
                stack.pop_back();

                if (stack.empty()) {
                    source->tokenTree = std::move(temp);
                    return;
                }

                stack.back().tokens.emplace_back(std::move(temp), std::move(commentStack.back()));
                commentStack.pop_back();
                pending_comments = {};
                continue;
            }

            if (token.isClosing()) {
                if (stack.back().left.expectedClosing() == TokenType::CloseAngle) {
                    auto temp = std::move(stack.back());
                    stack.pop_back();

                    temp.left.type = TokenType::LessThan;
                    stack.back().tokens.emplace_back(temp.left, std::move(commentStack.back()));
                    commentStack.pop_back();

                    auto &tokens = stack.back().tokens;
                    tokens.insert(tokens.end(), std::make_move_iterator(temp.tokens.begin()),
                                  std::make_move_iterator(temp.tokens.end()));
                    peeked = token;
                    continue;
                }

                if (token.type == TokenType::CloseAngle) {
                    token.type = TokenType::GreaterThan;
                    stack.back().tokens.emplace_back(token, std::move(pending_comments));
                    pending_comments = {};
                    continue;
                }


                if (std::ranges::find_if(stack, [&](const TokenTree &t) {
                    return t.left.expectedClosing() == token.type;
                }) != stack.end()) {
                    while (stack.back().left.expectedClosing() != token.type) {
                        stack.back().right = LexerErr::UnclosedTokenTree(token, TokenTypeStringQuoted(stack.back().left.expectedClosing()));

                        auto temp = std::move(stack.back());
                        stack.pop_back();
                        stack.back().tokens.emplace_back(std::move(temp), std::move(commentStack.back()));
                        commentStack.pop_back();
                    }

                    stack.back().right = token;
                    auto temp = std::move(stack.back());
                    stack.pop_back();

                    if (stack.empty()) {
                        source->tokenTree = std::move(temp);
                        return;
                    }

                    stack.back().tokens.emplace_back(std::move(temp), std::move(commentStack.back()));
                    commentStack.pop_back();
                } else {
                    stack.back().tokens.emplace_back(LexerErr::UnexpectedInput(token, TokenTypeStringQuoted(stack.back().left.expectedClosing())),
                                                     std::move(pending_comments));
                }
                pending_comments = {};
                continue;
            }

            if (stack.back().left.type == TokenType::OpenAngle && !token.isAllowedInAngleBrackets()) {
                auto temp = std::move(stack.back());
                stack.pop_back();

                temp.left.type = TokenType::LessThan;
                stack.back().tokens.emplace_back(temp.left, std::move(commentStack.back()));
                commentStack.pop_back();

                auto &tokens = stack.back().tokens;
                tokens.insert(tokens.end(), std::make_move_iterator(temp.tokens.begin()),
                              std::make_move_iterator(temp.tokens.end()));
            }

            if (token.type == TokenType::Destructures) {
                if (peekToken().isToken(TokenType::Into)) {
                    auto intoToken = nextToken();
                    token.type = TokenType::DestructuresInto;
                    token.end = intoToken.getEnd();
                }
            }

            stack.back().tokens.emplace_back(token, std::move(pending_comments));
            pending_comments = {};
        }
    }

    TokenResult Lexer::nextToken() {
        peekToken(); // ensure that a token has been read and is available in `peeked`
        position = peeked->getEnd() - source->offset;
        auto token = std::move(*peeked);
        peeked = {};
        return token;
    }

    void Lexer::consumeWhitespace() {
        const auto length = source->text.length();
        while (position < length) {
            if (
                    source->text[position] == ' '
                    || source->text[position] == '\t'
                    ) {
                position += 1;
            } else if (source->text[position] == '\n') {
                source->addLineBreak(position);
                position += 1;
            } else {
                break;
            }
        }
    }

    const TokenResult &Lexer::peekToken() {
        if (peeked.has_value()) {
            return *peeked;
        }

        consumeWhitespace();

        if (position >= source->text.length()) {
            peeked = Token(TokenType::Eof, position, position);
            return *peeked;
        }

        const auto ch = source->text[position];
        switch (ch) {
            case '{':
                peeked = Token(TokenType::OpenCurly, source->offset + position, source->offset + position + 1);
                return *peeked;
            case '}':
                peeked = Token(TokenType::CloseCurly, source->offset + position, source->offset + position + 1);
                return *peeked;
            case '(':
                peeked = Token(TokenType::OpenParen, source->offset + position, source->offset + position + 1);
                return *peeked;
            case ')':
                peeked = Token(TokenType::CloseParen, source->offset + position, source->offset + position + 1);
                return *peeked;
            case '<':
                peeked = Token(TokenType::OpenAngle, source->offset + position, source->offset + position + 1);
                return *peeked;
            case '>':
                peeked = Token(TokenType::CloseAngle, source->offset + position, source->offset + position + 1);
                return *peeked;
            case ';':
                peeked = Token(TokenType::Semicolon, source->offset + position, source->offset + position + 1);
                return *peeked;
            case ',':
                peeked = Token(TokenType::Comma, source->offset + position, source->offset + position + 1);
                return *peeked;
            case '~':
                peeked = Token(TokenType::Tilde, source->offset + position, source->offset + position + 1);
                return *peeked;
            case ':':
                peeked = colonRule();
                return *peeked;
            case '-':
                peeked = dashRule();
                return *peeked;
            case '=':
                peeked = equalsRule();
                return *peeked;
            case '/':
                peeked = slashRule();
                return *peeked;

                //        case '"':
                //            return quotedStringRule();
            default:
                break;
        }


        peeked = identifierRule();
        return *peeked;
    }

    TokenResult Lexer::colonRule() const {
        if (position + 1 < source->text.length() && source->text[position + 1] == ':') {
            return Token(TokenType::PathSeparator, source->offset + position, source->offset + position + 2);
        }
        return Token(TokenType::Colon, source->offset + position, source->offset + position + 1);
    }

    TokenResult Lexer::dashRule() const {
        if (position + 1 < source->text.length() && source->text[position + 1] == '>') {
            return Token(TokenType::DashArrow, source->offset + position, source->offset + position + 2);
        }
        return LexerErr::UnexpectedInput(source->offset + position, source->offset + position + 1);
    }

    TokenResult Lexer::equalsRule() const {
        if (position + 1 < source->text.length() && source->text[position + 1] == '>') {
            return Token(TokenType::EqualArrow, source->offset + position, source->offset + position + 2);
        }
        return Token(TokenType::Equals, source->offset + position, source->offset + position + 1);
    }

    TokenResult Lexer::slashRule() const {
        if (position + 1 < source->text.length()) {
            const auto remaining = source->text.length() - position;
            uint32_t offset = 2;
            if (source->text[position + 1] == '/') {
                while (offset < remaining) {
                    if (source->text[position + offset] == '\n') {
                        break;
                    }
                    offset += 1;
                }
                return Token(TokenType::LineComment, source->offset + position, source->offset + position + offset);
            }
            if (source->text[position + 1] == '*') {
                while (offset < remaining) {
                    if (offset + 1 < remaining && source->text[position + offset] == '*' && source->text[
                                                                                                    position + offset + 1] == '/') {
                        return Token(TokenType::MultiLineComment, source->offset + position,
                                     source->offset + position + offset + 2);
                    }
                    if (source->text[position + offset] == '\n') {
                        source->addLineBreak(offset);
                    }
                    offset += 1;
                }
                return LexerErr::UnexpectedEndOfInput(source->offset + position + offset, "*/");
            }
        }
        return LexerErr::UnexpectedInput(source->offset + position, source->offset + position + 1);
    }

    TokenResult Lexer::identifierRule() const {
        auto ch = source->text[position];
        if ((ch >= 'a' && ch <= 'z')
            || (ch >= 'A' && ch <= 'Z')
            || ch == '_'
            || ch == '@'
                ) {
            uint32_t offset = 1;
            const uint32_t remaining = source->text.length() - position;

            while (offset < remaining) {
                ch = source->text[position + offset];
                if ((ch >= 'a' && ch <= 'z')
                    || (ch >= 'A' && ch <= 'Z')
                    || (ch >= '0' && ch <= '9')
                    || ch == '_'
                        ) {
                    offset = offset + 1;
                } else {
                    break;
                }
            }

            const auto text = source->getText(position, position + offset);
            auto tokenType = TokenType::Identifier;

            if (text == "_") {
                tokenType = TokenType::Discard;
            } else if (text == "@") {
                return LexerErr::InvalidIdentifier(source->offset + position, source->offset + position + 1);
            } else if (text == "use") {
                tokenType = TokenType::Use;
            } else if (text == "mod") {
                tokenType = TokenType::Mod;
            } else if (text == "enum") {
                tokenType = TokenType::Enum;
            } else if (text == "interface") {
                tokenType = TokenType::Interface;
            } else if (text == "fn") {
                tokenType = TokenType::Fn;
            } else if (text == "struct") {
                tokenType = TokenType::Struct;
            } else if (text == "destructures") {
                tokenType = TokenType::Destructures;
            } else if (text == "into") {
                tokenType = TokenType::Into;
            } else if (text == "alias") {
                tokenType = TokenType::Alias;
            } else if (text == "let") {
                tokenType = TokenType::Let;
            } else if (text == "pub") {
                tokenType = TokenType::Pub;
            } else if (text == "where") {
                tokenType = TokenType::Where;
            } else if (text == "get") {
                tokenType = TokenType::Get;
            } else if (text == "set") {
                tokenType = TokenType::Set;
            } else if (text == "mut") {
                tokenType = TokenType::Mut;
            } else if (text == "ref") {
                tokenType = TokenType::Ref;
            } else if (text == "impl") {
                tokenType = TokenType::Impl;
            } else if (text == "on") {
                tokenType = TokenType::On;
            } else if (text == "static") {
                tokenType = TokenType::Static;
            } else if (text == "for") {
                tokenType = TokenType::For;
            } else if (text == "value") {
                tokenType = TokenType::Value;
            } else if (text == "as") {
                tokenType = TokenType::As;
            }

            return Token(tokenType, source->offset + position, source->offset + position + offset);
        }
        return LexerErr::UnexpectedInput(source->offset + position, source->offset + position + 1);
    }

    TokenResult Lexer::quotedStringRule() {
        return Token(TokenType::Error, 0, 0); // TODO not implemented
    }

}

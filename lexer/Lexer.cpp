//
// Created by zoe on 06.09.24.
//

#include "Lexer.h"

TokenTree Lexer::tokenize() {
    std::vector<TokenTree> stack;
    stack.emplace_back(Token(TokenType::Bof, source->offset, source->offset));

    while (true) {
        auto tokenResult = nextToken();

        if (tokenResult.isError()) {
            stack.back().tokens.emplace_back(tokenResult);
            continue;
        }

        auto token = tokenResult.get();

        if (token.isOpening()) {
            stack.emplace_back(token);
            continue;
        }

        if (token.type == stack.back().left.expectedClosing()) {
            stack.back().right = tokenResult;
            auto temp = stack.back();
            stack.pop_back();

            if (stack.empty()) {
                return temp;
            }

            stack.back().tokens.emplace_back(temp);
            continue;
        }

        if (token.isClosing()) {
            if (stack.back().left.expectedClosing() == TokenType::CloseAngle) {
                auto temp = stack.back();
                stack.pop_back();

                auto openToken = temp.left;
                openToken.type = TokenType::LessThan;
                stack.back().tokens.emplace_back(openToken);

                auto &tokens = stack.back().tokens;
                tokens.insert(tokens.end(), std::make_move_iterator(temp.tokens.begin()),
                              std::make_move_iterator(temp.tokens.end()));
                peeked = token;
                continue;
            }

            if (token.type == TokenType::CloseAngle) {
                token.type = TokenType::GreaterThan;
                stack.back().tokens.emplace_back(token);
                continue;
            }


            if (std::ranges::find_if(stack, [&](const TokenTree &t) {
                return t.left.expectedClosing() == token.type;
            }) != stack.end()) {
                while (stack.back().left.expectedClosing() != token.type) {
                    auto errorMsg = std::format(
                        "unclosed token tree, found {}, expected {}",
                        TokenTypeName(token.type),
                        TokenTypeName(stack.back().left.expectedClosing())
                    );
                    stack.back().right = LexerErr(
                        Token(TokenType::Error, token.start, token.start),
                        errorMsg
                    );

                    auto temp = stack.back();
                    stack.pop_back();
                    stack.back().tokens.emplace_back(temp);
                }

                stack.back().right = token;
                auto temp = stack.back();
                stack.pop_back();

                if (stack.empty()) {
                    return temp;
                }

                stack.back().tokens.emplace_back(temp);
            } else {
                auto errorMsg = std::format(
                    "unexpected {}, expected {}",
                    TokenTypeName(token.type),
                    TokenTypeName(stack.back().left.expectedClosing())
                );
                stack.back().tokens.emplace_back(LexerErr(token, errorMsg));
            }
            continue;
        }

        if (stack.back().left.type == TokenType::OpenAngle && !token.isAllowedInAngleBrackets()) {
            auto temp = stack.back();
            stack.pop_back();

            auto openToken = temp.left;
            openToken.type = TokenType::LessThan;
            stack.back().tokens.emplace_back(openToken);

            auto &tokens = stack.back().tokens;
            tokens.insert(tokens.end(), std::make_move_iterator(temp.tokens.begin()),
                          std::make_move_iterator(temp.tokens.end()));
        }

        stack.back().tokens.emplace_back(token);
    }
}

TokenResult Lexer::nextToken() {
    const auto result = peekToken();
    position = result.getEnd() - source->offset;
    peeked = {};
    return result;
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

TokenResult Lexer::peekToken() {
    if (peeked.has_value()) {
        return *peeked;
    }

    consumeWhitespace();

    if (position >= source->text.length()) {
        return Token(TokenType::Eof, position, position);
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
    return Token(TokenType::Colon, source->offset + position + 1, source->offset + position + 1);
}

TokenResult Lexer::dashRule() const {
    if (position + 1 < source->text.length() && source->text[position + 1] == '>') {
        return Token(TokenType::DashArrow, source->offset + position, source->offset + position + 2);
    }
    return LexerErr(
        Token(TokenType::Error, source->offset + position, source->offset + position + 1),
        "Unexpected character, expected > after -");
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
            return LexerErr(
                Token(TokenType::MultiLineComment, source->offset + position, source->offset + position + offset),
                "Unexpected end of input in multiline comment.");
        }
    }
    return LexerErr(
        Token(TokenType::Error, source->offset + position, source->offset + position + 1),
        "Unexpected /");
}

TokenResult Lexer::identifierRule() const {
    auto ch = source->text[position];
    if (ch >= 'a' && ch <= 'z'
        || ch >= 'A' && ch <= 'Z'
        || ch == '_'
        || ch == '@'
    ) {
        uint32_t offset = 1;
        const uint32_t remaining = source->text.length() - position;

        while (offset < remaining) {
            ch = source->text[position + offset];
            if (ch >= 'a' && ch <= 'z'
                || ch >= 'A' && ch <= 'Z'
                || ch >= '0' && ch <= '9'
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
            return LexerErr(
                Token(TokenType::Error, source->offset + position, source->offset + position + 1),
                "'@' is not a valid identifier or keyword.");
        } else if (text == "use") {
            tokenType = TokenType::Use;
        } else if (text == "mod") {
            tokenType = TokenType::Mod;
        } else if (text == "enum") {
            tokenType = TokenType::Enum;
        } else if (text == "interface") {
            tokenType = TokenType::Interface;
        } else if (text == "require") {
            tokenType = TokenType::Require;
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
        }

        return Token(tokenType, source->offset + position, source->offset + position + offset);
    }
    return LexerErr(
        Token(TokenType::Error, source->offset + position, source->offset + position + 1),
        "Unexpected character, expected valid identifier or keyword.");
}

TokenResult Lexer::quotedStringRule() {
    return Token(TokenType::Error, 0, 0); // TODO not implemented
}

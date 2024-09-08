//
// Created by zoe on 06.09.24.
//

#include "TokenType.h"

#include <map>


static std::map<TokenType, std::string> TokenTypeNames = {
    {TokenType::Bof, "Bof"},
    {TokenType::Eof, "Eof"},
    {TokenType::Identifier, "Identifier"},
    {TokenType::Discard, "Discard"},
    {TokenType::Use, "Use"},
    {TokenType::Mod, "Mod"},
    {TokenType::Enum, "Enum"},
    {TokenType::Interface, "Interface"},
    {TokenType::Require, "Require"},
    {TokenType::Fn, "Fn"},
    {TokenType::Struct, "Struct"},
    {TokenType::Destructures, "Destructures"},
    {TokenType::Into, "Into"},
    {TokenType::Alias, "Alias"},
    {TokenType::Let, "Let"},
    {TokenType::Pub, "Pub"},
    {TokenType::PathSeparator, "PathSeparator"},
    {TokenType::DashArrow, "DashArrow"},
    {TokenType::EqualArrow, "EqualArrow"},
    {TokenType::Equals, "Equals"},
    {TokenType::Colon, "Colon"},
    {TokenType::Semicolon, "Semicolon"},
    {TokenType::Comma, "Comma"},
    {TokenType::OpenCurly, "OpenCurly"},
    {TokenType::CloseCurly, "CloseCurly"},
    {TokenType::OpenParen, "OpenParen"},
    {TokenType::CloseParen, "CloseParen"},
    {TokenType::OpenAngle, "OpenAngle"},
    {TokenType::CloseAngle, "CloseAngle"},
    {TokenType::LessThan, "LessThan"},
    {TokenType::GreaterThan, "GreaterThan"},
    {TokenType::Error, "Error"},
    {TokenType::MultiLineComment, "MultiLineComment"},
    {TokenType::LineComment, "LineComment"},
};

std::string_view TokenTypeName(TokenType type) {
    return TokenTypeNames[type];
}

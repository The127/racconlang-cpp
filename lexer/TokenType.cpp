//
// Created by zoe on 06.09.24.
//

#include "TokenType.h"

#include <map>

namespace racc::lexer {

    static std::map<TokenType, std::pair<std::string, std::string>> TokenTypes = {
            {TokenType::Bof,              {"Bof",              {}}},
            {TokenType::Eof,              {"Eof",              {}}},
            {TokenType::Identifier,       {"Identifier",       {}}},
            {TokenType::Discard,          {"Discard",          "_"}},
            {TokenType::Use,              {"Use",              "use"}},
            {TokenType::Mod,              {"Mod",              "mod"}},
            {TokenType::Enum,             {"Enum",             "enum"}},
            {TokenType::Interface,        {"Interface",        "interface"}},
            {TokenType::Fn,               {"Fn",               "fn"}},
            {TokenType::Struct,           {"Struct",           "struct"}},
            {TokenType::Destructures,     {"Destructures",     "destructures"}},
            {TokenType::Into,             {"Into",             "into"}},
            {TokenType::DestructuresInto, {"DestructuresInto", "destructures into"}},
            {TokenType::Alias,            {"Alias",            "alias"}},
            {TokenType::Let,              {"Let",              "let"}},
            {TokenType::Pub,              {"Pub",              "pub"}},
            {TokenType::Where,            {"Where",            "where"}},
            {TokenType::Get,              {"Get",              "get"}},
            {TokenType::Set,              {"Set",              "set"}},
            {TokenType::Mut,              {"Mut",              "mut"}},
            {TokenType::Ref,              {"Ref",              "ref"}},
            {TokenType::Impl,             {"Impl",             "impl"}},
            {TokenType::On,               {"on",               "on"}},
            {TokenType::Static,           {"Static",           "static"}},
            {TokenType::For,              {"For",              "for"}},
            {TokenType::Value,            {"Value",            "value"}},
            {TokenType::As,               {"As",               "as"}},
            {TokenType::PathSeparator,    {"PathSeparator",    "::"}},
            {TokenType::DashArrow,        {"DashArrow",        "->"}},
            {TokenType::EqualArrow,       {"EqualArrow",       "=>"}},
            {TokenType::Tilde,            {"Tilde",            "~"}},
            {TokenType::Equals,           {"Equals",           "="}},
            {TokenType::Colon,            {"Colon",            ":"}},
            {TokenType::Semicolon,        {"Semicolon",        ";"}},
            {TokenType::Comma,            {"Comma",            ","}},
            {TokenType::OpenCurly,        {"OpenCurly",        "{"}},
            {TokenType::CloseCurly,       {"CloseCurly",       "}"}},
            {TokenType::OpenParen,        {"OpenParen",        "("}},
            {TokenType::CloseParen,       {"CloseParen",       ")"}},
            {TokenType::OpenAngle,        {"OpenAngle",        "<"}},
            {TokenType::CloseAngle,       {"CloseAngle",       ">"}},
            {TokenType::LessThan,         {"LessThan",         "<"}},
            {TokenType::GreaterThan,      {"GreaterThan",      ">"}},
            {TokenType::Error,            {"Error",            {}}},
            {TokenType::MultiLineComment, {"MultiLineComment", {}}},
            {TokenType::LineComment,      {"LineComment",      {}}},
    };

    std::string_view TokenTypeName(TokenType type) {
        return TokenTypes[type].first;
    }


    std::string_view TokenTypeString(TokenType type) {
        return TokenTypes[type].second;
    }

    std::string TokenTypeStringQuoted(TokenType type) {
        if (!TokenTypes[type].second.empty()) {
            return "`" + TokenTypes[type].second + "`";
        }
        return {};
    }

}

//
// Created by zoe on 06.09.24.
//

#pragma once

#include <string>

enum class TokenType {
    Bof,            // beginning of file
    Eof,            // end of file

    Identifier,
    Discard,        // _

    Use,            // use
    Mod,            // mod
    Enum,           // enum
    Interface,      // interface
    Require,        // require
    Fn,             // fn
    Struct,         // struct
    Destructures,   // destructures
    Into,           // into
    Alias,          // alias
    Let,            // let
    Pub,            // pub

    Colon,          // :
    PathSeparator,  // ::

    DashArrow,      // ->

    Equals,         // =
    EqualArrow,     // =>

    Semicolon,      // ;
    Comma,          // ,

    OpenCurly,      // {
    CloseCurly,     // }
    OpenParen,      // (
    CloseParen,     // )
    OpenAngle,      // <
    CloseAngle,     // >

    LessThan,       // <
    GreaterThan,    // >

    LineComment,    // //
    MultiLineComment, // /* ... */

    Error,
};

std::string_view TokenTypeName(TokenType type);
std::string_view TokenTypeString(TokenType type);
std::string TokenTypeStringQuoted(TokenType type);
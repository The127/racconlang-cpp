//
// Created by zoe on 07.09.24.
//


#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "lexer/Token.h"
#include "lexer/TokenResult.h"
#include "lexer/TokenTree.h"


enum ErrorCode {
    MissingSemicolon,
    MissingStructBody,
    UseAfterMod,
    UseIsMissingPath,
    PathHasTrailingSeparator,
    WrongOpener,
    WrongCloser,
    UnexpectedToken,
    UnexpectedEndOfInput,
    DuplicateModifier,
    InvalidModifier,
    MissingDeclarationName,
    MissingPropertyName,
    MissingPropertyType,
    InvalidGenericConstraint,
    MissingEnumBody,
    EnumMemberExpected,
    UnclosedEnumBody,
    MissingComma,
    MissingSignature,
    FnSignatureMissingParams,
    InvalidSignature,
    ParameterNameMissing,
    ParameterTypeMissing,
    MissingInterfaceBody,
    RefAlreadyImpliesMut,
    MissingMethodName,
    MissingMethodReturnType,
    MissingGetterName,
    MissingGetterReturnType,
    MissingSetterName,
    MissingSetterParam,
    TooManySetterParams,
    MissingVariableType,
};

class ErrorLabel {
public:
    std::string text;
    uint64_t start;
    uint64_t end;

    ErrorLabel(std::string text, uint64_t start, uint64_t end);
};

class CompilerError {
public:
    ErrorCode code;
    uint64_t position;
    std::vector<ErrorLabel> labels;
    std::optional<std::string> note;


    CompilerError(ErrorCode code, uint64_t position);
    CompilerError(ErrorCode code, const Token &token);

    CompilerError(CompilerError&&) noexcept;
    CompilerError& operator=(CompilerError&&) noexcept;
    ~CompilerError();

    void addLabel(const ErrorLabel &label);

    void addLabel(const std::string &text, uint64_t start, uint64_t end);

    void addLabel(const std::string &text, const Token &startToken, const Token &endToken);

    void addLabel(const std::string &text, const Token &token);

    void addLabel(const std::string &text, const TokenResult &result);

    void addLabel(const std::string &text, const TokenResult &startResult, const TokenResult &endResult);

    void addLabel(const std::string &text, const TokenTree &tree);

    void addLabel(const std::string &text, const TokenTreeNode &node);

    void setNote(const std::string& note);
};

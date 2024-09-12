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
    UseAfterMod,
    UseIsMissingPath,
    PathHasTrailingSeparator,
    WrongOpener,
    WrongCloser,
    UnexpectedToken,
    DuplicateModifier,
    InvalidModifier,
    MissingDeclarationName,
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

    ErrorLabel(std::string text, const uint64_t start, const uint64_t end)
        : text(std::move(text)), start(start), end(end) {
    }
};

class CompilerError {
public:
    ErrorCode code;
    uint64_t position;
    std::vector<ErrorLabel> labels;
    std::optional<std::string> note;


    CompilerError(const ErrorCode code, const uint64_t position)
        : code(code), position(position) {
    }

    CompilerError(const ErrorCode code, const Token &token)
        : code(code), position(token.start) {
    }

    void addLabel(const ErrorLabel &label) {
        labels.push_back(label);
    }

    void addLabel(const std::string &text, const uint64_t start, const uint64_t end) {
        labels.emplace_back(text, start, end);
    }

    void addLabel(const std::string &text, const Token &startToken, const Token &endToken) {
        labels.emplace_back(text, startToken.start, endToken.end);
    }

    void addLabel(const std::string &text, const Token &token) {
        addLabel(text, token, token);
    }

    void addLabel(const std::string &text, const TokenResult &result) {
        addLabel(text, result.getOrErrorToken());
    }

    void addLabel(const std::string &text, const TokenResult &startResult, const TokenResult &endResult) {
        addLabel(text, startResult.getOrErrorToken(), endResult.getOrErrorToken());
    }

    void addLabel(const std::string &text, const TokenTree &tree) {
        addLabel(text, tree.left, tree.right.getOrErrorToken());
    }

    void addLabel(const std::string &text, const TokenTreeNode &node) {
        addLabel(text, node.getStart(), node.getEnd());
    }

    void setNote(const std::string& note) {
        this->note = note;
    }
};

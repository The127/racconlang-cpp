//
// Created by zoe on 14.09.24.
//


#pragma once
#include <string>
#include <utility>

enum class ErrorCode {
    MissingSemicolon,
    MissingStructBody,
    UseAfterMod,
    UseIsMissingPath,
    EmptyPath, //TODO: add
    MissingPathSeparator,
    MissingModulePath,
    WrongOpener,
    WrongCloser,
    UnexpectedToken,
    UnexpectedEndOfInput,
    MissingMethodParams, //TODO
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
    MissingEquals,
    MissingSignature,
    FnSignatureMissingParams,
    InvalidSignature,
    ParameterNameMissing,
    ParameterTypeMissing,
    MissingInterfaceBody,
    MissingInterfaceMember,
    RefAlreadyImpliesMut,
    MissingMethodName,
    MissingMethodReturnType,
    MissingGetterName,
    MissingGetterReturnType,
    MissingGetterParam,
    TooManyGetterParams,
    MissingSetterName,
    MissingSetterParam,
    TooManySetterParams,
    MissingVariableType,
    MissingAliasType,
    MissingColon,
};

const std::pair<std::string, std::string>& ErrorInfo(ErrorCode code);

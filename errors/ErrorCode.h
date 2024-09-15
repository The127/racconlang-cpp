//
// Created by zoe on 14.09.24.
//


#pragma once
#include <string>
#include <utility>

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
    MissingEquals,
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

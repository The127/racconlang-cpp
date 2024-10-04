//
// Created by zoe on 14.09.24.
//


#pragma once

#include "predeclare.h"

#include <string>
#include <utility>

enum class racc::errors::ErrorCode {
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
    MissingReturnType,
    MissingGetterName,
    MissingGetterParam,
    TooManyGetterParams,
    MissingSetterName,
    MissingSetterParam,
    TooManySetterParams,
    MissingVariableType,
    MissingAliasType,
    MissingColon,
    MissingAsName,
    DuplicateTypeDeclaration,
    UnknownType,
    InaccessibleType,
    PublicMemberOnNonPublicStruct,
    DuplicateMemberName,

    _end,
};

namespace racc::errors {
    const std::pair<std::string, std::string> &ErrorInfo(ErrorCode code);
}
//
// Created by zoe on 14.09.24.
//

#include "ErrorCode.h"
#include "InternalError.h"

#include <map>
#include <string>

namespace racc::errors {

    static std::map<ErrorCode, std::pair<std::string, std::string> > ErrorInfos = {
            {ErrorCode::MissingSemicolon,         {"EXX0001", "missing semicolon"}},
            {ErrorCode::MissingComma,             {"EXX0002", "missing comma"}},
            {ErrorCode::WrongOpener,              {"EXX0003", "wrong open token"}},
            {ErrorCode::WrongCloser,              {"EXX0004", "wrong close token"}},
            {ErrorCode::UnexpectedToken,          {"EXX0005", "unexpected token"}},
            {ErrorCode::UnexpectedEndOfInput,     {"EXX0006", "unexpected end of input"}},
            {ErrorCode::DuplicateModifier,        {"EXX0007", "duplicate modifier"}},
            {ErrorCode::InvalidModifier,          {"EXX0008", "invalid modifier"}},
            {ErrorCode::InvalidGenericConstraint, {"EXX0010", "invalid generic constraint"}},
            {ErrorCode::MissingEquals,            {"EXX0011", "missing equals"}},
            {ErrorCode::MissingColon,             {"EXX0012", "missing colon"}},

            {ErrorCode::UseAfterMod,              {"EUS0001", "use declaration after module declaration"}},
            {ErrorCode::UseIsMissingPath,         {"EUS0002", "use is missing path"}},

            {ErrorCode::InvalidSignature,         {"ESI0001", "invalid signature"}},
            {ErrorCode::MissingSignature,         {"ESI0002", "missing signature"}},
            {ErrorCode::FnSignatureMissingParams, {"ESI0003", "function signature missing params"}},

            {ErrorCode::ParameterNameMissing,     {"EPA0001", "parameter name missing"}},
            {ErrorCode::ParameterTypeMissing,     {"EPA0002", "parameter type missing"}},
            {ErrorCode::RefAlreadyImpliesMut,     {"EPA0003", "ref already implies mut"}},

            {ErrorCode::MissingMethodName,        {"EME0001", "missing method name"}},
            {ErrorCode::MissingReturnType,        {"EME0002", "missing return type"}},
            {ErrorCode::MissingGetterName,        {"EME0003", "missing getter name"}},
            {ErrorCode::MissingSetterName,        {"EME0005", "missing setter name"}},
            {ErrorCode::MissingSetterParam,       {"EME0006", "missing setter parameter"}},
            {ErrorCode::TooManySetterParams,      {"EME0007", "too many setter parameters"}},
            {ErrorCode::MissingSetterParam,       {"EME0008", "missing getter parameter"}},
            {ErrorCode::TooManySetterParams,      {"EME0009", "too many getter parameters"}},

            {ErrorCode::MissingDeclarationName,   {"EDE0001", "missing declaration name"}},

            {ErrorCode::DuplicateTypeDeclaration, {"ETY0001", "duplicate type declaration"}},
            {ErrorCode::UnknownType,              {"ETY0002", "unknown type"}},

            {ErrorCode::MissingStructBody,        {"EST0001", "missing struct body"}},
            {ErrorCode::MissingPropertyName,      {"EST0002", "missing property name"}},
            {ErrorCode::MissingPropertyType,      {"EST0003", "missing property type"}},

            {ErrorCode::MissingEnumBody,          {"EEN0001", "missing enum body"}},
            {ErrorCode::UnclosedEnumBody,         {"EEN0002", "unclosed enum body"}},
            {ErrorCode::EnumMemberExpected,       {"EEN0003", "expected enum member"}},

            {ErrorCode::MissingInterfaceBody,     {"EIN0001", "missing interface body"}},

            {ErrorCode::MissingVariableType,      {"EVA0001", "missing variable type"}},

            {ErrorCode::MissingAliasType,         {"EAL0001", "missing alias type"}},
    };

    const std::pair<std::string, std::string> &ErrorInfo(ErrorCode code) {
        COMPILER_ASSERT(code < ErrorCode::_end, "error code is not defined");
        return ErrorInfos[code];
    }

}
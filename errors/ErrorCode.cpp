//
// Created by zoe on 14.09.24.
//

#include "ErrorCode.h"

#include <map>
#include <string>


static std::map<ErrorCode, std::pair<std::string, std::string> > ErrorInfos = {
    {MissingSemicolon, {"EXX0001", "missing semicolon"}},
    {MissingComma, {"EXX0002", "missing comma"}},
    {WrongOpener, {"EXX0003", "wrong open token"}},
    {WrongCloser, {"EXX0004", "wrong close token"}},
    {UnexpectedToken, {"EXX0005", "unexpected token"}},
    {UnexpectedEndOfInput, {"EXX0006", "unexpected end of input"}},
    {DuplicateModifier, {"EXX0007", "duplicate modifier"}},
    {InvalidModifier, {"EXX0008", "invalid modifier"}},
    {PathHasTrailingSeparator, {"EXX0009", "invalid trailing path separatr"}},
    {InvalidGenericConstraint, {"EXX0010", "invalid generic constraint"}},

    {UseAfterMod, {"EUS0001", "use declaration after module declaration"}},
    {UseIsMissingPath, {"EUS0002", "use is missing path"}},

    {InvalidSignature, {"ESI0001", "invalid signature"}},
    {MissingSignature, {"ESI0002", "missing signature"}},
    {FnSignatureMissingParams, {"ESI0003", "function signature missing params"}},

    {ParameterNameMissing, {"EPA0001", "parameter name missing"}},
    {ParameterTypeMissing, {"EPA0002", "parameter type missing"}},
    {RefAlreadyImpliesMut, {"EPA0003", "ref already implies mut"}},

    {MissingMethodName, {"EME0001", "missing method name"}},
    {MissingMethodReturnType, {"EME0002", "missing method return type"}},
    {MissingGetterName, {"EME0003", "missing getter name"}},
    {MissingGetterReturnType, {"EME0004", "missing getter return type"}},
    {MissingSetterName, {"EME0005", "missing setter name"}},
    {MissingSetterParam, {"EME0006", "missing setter parameter"}},
    {TooManySetterParams, {"EME0007", "too many setter parameters"}},

    {MissingDeclarationName, {"EDE0001", "missing declaration name"}},

    {MissingStructBody, {"EST0001", "missing struct body"}},
    {MissingPropertyName, {"EST0002", "missing property name"}},
    {MissingPropertyType, {"EST0003", "missing property type"}},

    {MissingEnumBody, {"EEN0001", "missing enum body"}},
    {UnclosedEnumBody, {"EEN0002", "unclosed enum body"}},
    {EnumMemberExpected, {"EEN0003", "expected enum member"}},

    {MissingInterfaceBody, {"EIN0001", "missing interface body"}},

    {MissingVariableType, {"EVA0001", "missing variable type"}},
};

const std::pair<std::string, std::string> & ErrorInfo(ErrorCode code) {
    return ErrorInfos[code];
}

#pragma once

namespace racc {
    namespace ast {
        class AliasDeclaration;
        class ConstraintDeclaration;
        class ConstructorDeclaration;
        class DestructureDeclaration;
        class EnumDeclaration;
        class EnumMemberDeclaration;
        class FileUses;
        class FunctionDeclaration;
        class FunctionSignature;
        class GenericConstraintBase;
        class Identifier;
        class ImplBlock;
        class ImplGetter;
        class ImplMethod;
        class ImplSetter;
        class InterfaceConstraint;
        class InterfaceDeclaration;
        class InterfaceGetterDeclaration;
        class InterfaceMethodDeclaration;
        class InterfaceSetterDeclaration;
        class ModuleDeclaration;
        class ModuleVariableDeclaration;
        class NamelessParameter;
        class Node;
        class Parameter;
        class Path;
        class PropertyDeclaration;
        class ReturnType;
        class Signature;
        class StructDeclaration;
        class TupleSignature;
        class TypeSignature;
        class UseMap;
        class UseNode;
    }

    namespace errors {
        class CompilerError;
        class ConsoleErrorHandler;
        enum class ErrorCode;
        class ErrorContext;
        class ErrorHandler;
        class ErrorLabel;
        class ErrorLabelGroup;
        class SimpleErrorLabel;
    }

    namespace lexer {
        class Lexer;
        class LexerErr;
        enum class LexerErrReason;
        class Token;
        class TokenResult;
        class TokenTree;
        class TokenTreeNode;
        enum class TokenType;
    }

    namespace parser {
        class Parser;
        class TokenTreeIterator;
    }

    namespace registry {
        class Alias;
        class BuiltinType;
        class Enum;
        class EnumMember;
        class FunctionType;
        class Interface;
        class InterfaceGetter;
        class InterfaceMethod;
        class InterfaceSetter;
        class Module;
        class ModuleRegistry;
        class Parameter;
        enum class ParameterMode;
        class Struct;
        class StructMember;
        class TupleType;
        class TypeRef;
        class TypeVar;
    }

    namespace sourcemap {
        class Location;
        class Source;
        class SourceMap;
    }
}
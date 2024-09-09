cmake_minimum_required(VERSION 3.29)
project(RaccoonLang)

set(CMAKE_CXX_STANDARD 26)

add_executable(RaccoonLang main.cpp
        lexer/Lexer.cpp
        lexer/Lexer.h
        lexer/TokenTree.cpp
        lexer/TokenTree.h
        lexer/Token.cpp
        lexer/Token.h
        sourceMap/Location.cpp
        sourceMap/Location.h
        sourceMap/SourceMap.cpp
        sourceMap/SourceMap.h
        sourceMap/Source.cpp
        sourceMap/Source.h
        lexer/TokenType.cpp
        lexer/TokenType.h
        lexer/LexerErr.cpp
        lexer/LexerErr.h
        ast/Node.cpp
        ast/Node.h
        ast/UseNode.cpp
        ast/UseNode.h
        ast/ModuleDeclaration.cpp
        ast/ModuleDeclaration.h
        ast/StructDeclaration.cpp
        ast/StructDeclaration.h
        ast/EnumDeclaration.cpp
        ast/EnumDeclaration.h
        ast/InterfaceDeclaration.cpp
        ast/InterfaceDeclaration.h
        ast/FunctionDeclaration.cpp
        ast/FunctionDeclaration.h
        ast/AliasDeclaration.cpp
        ast/AliasDeclaration.h
        ast/ModuleVariableDeclaration.cpp
        ast/ModuleVariableDeclaration.h
        ast/ConstraintDeclaration.cpp
        ast/ConstraintDeclaration.h
        ast/GenericConstraint.cpp
        ast/GenericConstraintBase.h
        ast/InterfaceConstraint.cpp
        ast/InterfaceConstraint.h
        ast/BaseSignature.cpp
        ast/SignatureBase.h
        ast/TypeSignature.cpp
        ast/TypeSignature.h
        ast/EnumMemberDeclaration.cpp
        ast/EnumMemberDeclaration.h
        ast/InterfaceMethodDeclaration.cpp
        ast/InterfaceMethodDeclaration.h
        ast/PropertyDeclaration.cpp
        ast/PropertyDeclaration.h
        ast/Path.cpp
        ast/Path.h
        ast/Identifier.cpp
        ast/Identifier.h
        parser/Parser.cpp
        parser/Parser.h
        CompilerError.cpp
        CompilerError.h
        InternalError.cpp
        InternalError.h
        lexer/TokenTreeNode.cpp
        lexer/TokenTreeNode.h
        lexer/TokenResult.cpp
        lexer/TokenResult.h
        utils/StringUtils.h
        utils/NodeUtils.h)

target_include_directories(RaccoonLang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
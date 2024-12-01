cmake_minimum_required(VERSION 3.29)
project(RaccoonLang)

set(CMAKE_CXX_STANDARD 26)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

find_package(LLVM REQUIRED CONFIG)

add_library(racc STATIC
        lexer/Lexer.cpp
        lexer/TokenTree.cpp
        lexer/Token.cpp
        sourceMap/Location.cpp
        sourceMap/SourceMap.cpp
        sourceMap/Source.cpp
        lexer/TokenType.cpp
        lexer/LexerErr.cpp
        ast/Node.cpp
        ast/UseNode.cpp
        ast/ModuleDeclaration.cpp
        ast/StructDeclaration.cpp
        ast/EnumDeclaration.cpp
        ast/InterfaceDeclaration.cpp
        ast/FunctionDeclaration.cpp
        ast/AliasDeclaration.cpp
        ast/ModuleVariableDeclaration.cpp
        ast/ConstraintDeclaration.cpp
        ast/GenericConstraintBase.cpp
        ast/InterfaceConstraint.cpp
        ast/Signature.cpp
        ast/TypeSignature.cpp
        ast/EnumMemberDeclaration.cpp
        ast/InterfaceMethodDeclaration.cpp
        ast/PropertyDeclaration.cpp
        ast/Path.cpp
        ast/Identifier.cpp
        parser/Parser.cpp
        errors/CompilerError.cpp
        errors/InternalError.cpp
        lexer/TokenTreeNode.cpp
        lexer/TokenResult.cpp
        ast/FileUses.cpp
        ast/Parameter.cpp
        ast/NamelessParameter.cpp
        ast/FunctionSignature.cpp
        ast/TupleSignature.cpp
        ast/InterfaceGetter.cpp
        ast/InterfaceSetter.cpp
        ast/ReturnType.cpp
        errors/ErrorHandler.cpp
        errors/ErrorCode.cpp
        errors/ConsoleErrorHandler.cpp
        errors/ErrorLabelGroup.cpp
        errors/SimpleErrorLabel.cpp
        errors/ErrorLabel.cpp
        ast/ImplBlock.cpp
        ast/DestructureDeclaration.cpp
        ast/ConstructorDeclaration.cpp
        ast/ImplGetter.cpp
        ast/ImplSetter.cpp
        ast/ImplMethod.cpp
        errors/ErrorContext.cpp
        registry/ModuleRegistry.cpp
        registry/Module.cpp
        registry/Struct.cpp
        registry/Enum.cpp
        registry/Alias.cpp
        registry/Interface.cpp
        registry/FunctionType.cpp
        registry/TupleType.cpp
        ast/UseMap.cpp
        registry/TypeVar.cpp
        registry/StructMember.cpp
        registry/EnumMember.cpp
        registry/TypeRef.cpp
        registry/BuiltinType.cpp
        registry/InterfaceGetter.cpp
        registry/InterfaceSetter.cpp
        registry/InterfaceMethod.cpp
        registry/Parameter.cpp
        registry/Id.cpp
        registry/Id.h
)


target_include_directories(racc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(racc PUBLIC ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
target_compile_definitions(racc PUBLIC ${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(LLVM_LIBS Support Core)
target_link_libraries(racc PUBLIC ${LLVM_LIBS})

target_compile_options(racc PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Wno-unused-variable
        -Wno-unused-parameter
        -Wno-unused-function
        -Wno-unused-but-set-variable
        -Werror
)
target_link_libraries(racc PRIVATE stdc++exp)

add_executable(racc_exe main.cpp)
target_link_libraries(racc_exe PRIVATE racc)
set_target_properties(racc_exe PROPERTIES OUTPUT_NAME racc)

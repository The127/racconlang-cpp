//
// Created by zoe on 07.09.24.
//


#pragma once

#include "predeclare.h"

#include "errors/CompilerError.h"
#include "TokenTreeIterator.h"

#include <functional>
#include <memory>

class racc::parser::Parser {
    using RecoverPredicate = std::function<bool(const lexer::TokenTreeNode &)>;
public:
    std::shared_ptr<sourcemap::Source> source;

    explicit Parser(const std::shared_ptr<sourcemap::Source> &source);

    Parser(const Parser &) = delete;

    Parser &operator=(const Parser &) = delete;

    Parser(Parser &&) noexcept;

    Parser &operator=(Parser &&) noexcept;

    ~Parser();

    std::vector<ast::ModuleDeclaration> parse();

private:
    std::vector<ast::UseNode> uses;

    std::vector<ast::ModuleDeclaration> modules;

    void addError(errors::CompilerError error);

    void parseFile();

    void useRule(TokenTreeIterator &it);

    void modRule(TokenTreeIterator &it);

    std::vector<lexer::Token> modifierRule(TokenTreeIterator &it, const RecoverPredicate &recoverPredicate);

    void validateModifiers(std::vector<lexer::Token> &modifiers, const std::vector<lexer::TokenType> &validTokenTypes);

    void enumRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::EnumMemberDeclaration enumMemberRule(TokenTreeIterator &it);

    void interfaceRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::InterfaceMethodDeclaration interfaceMethodRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::InterfaceGetterDeclaration interfaceGetterRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::InterfaceSetterDeclaration interfaceSetterRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);


    void structRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    std::vector<ast::ConstraintDeclaration> genericConstraintListRule(TokenTreeIterator &it, const RecoverPredicate &recoverPredicate);

    std::optional<ast::PropertyDeclaration> propertyDeclarationRule(TokenTreeIterator &it);

    void functionRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    void aliasRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    void moduleVariableRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    void declarationRule(TokenTreeIterator &it);

    void implRule(TokenTreeIterator &it);

    ast::ConstructorDeclaration constructorRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::DestructureDeclaration destructorRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::ImplMethod methodRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::ImplGetter implGetterRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::ImplSetter implSetterRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers);

    ast::Path pathRule(TokenTreeIterator &it, bool allowTrailing);

    std::optional<ast::ConstraintDeclaration> genericConstraintRule(TokenTreeIterator &it);

    std::optional<ast::InterfaceConstraint> interfaceConstraintRule(TokenTreeIterator &it);
//    std::unique_ptr<GenericConstraintBase> defaultConstraintRule(TokenTreeIterator& it);

    [[nodiscard]] ast::Identifier identifierRule(TokenTreeIterator &it) const;

    [[nodiscard]] ast::Signature signatureRule(TokenTreeIterator &it);

    [[nodiscard]] ast::TypeSignature typeSignatureRule(TokenTreeIterator &it);

    [[nodiscard]] ast::FunctionSignature functionSignatureRule(TokenTreeIterator &it);

    [[nodiscard]] ast::TupleSignature tupleSignatureRule(TokenTreeIterator &it);

    [[nodiscard]] std::optional<ast::ReturnType> returnTypeRule(TokenTreeIterator &it);

    [[nodiscard]] std::vector<ast::Identifier> identifierListRule(const lexer::TokenTree &list);

    [[nodiscard]] std::vector<ast::Parameter> parameterListRule(const lexer::TokenTree &list);

    [[nodiscard]] std::vector<ast::NamelessParameter> namelessParameterListRule(const lexer::TokenTree &list);

    [[nodiscard]] std::vector<ast::Signature> signatureListRule(const lexer::TokenTree &list);

    bool recoverTopLevel(TokenTreeIterator &it);

    bool recoverUntil(TokenTreeIterator &it, lexer::TokenType type);

    bool recoverUntil(TokenTreeIterator &it, std::vector<lexer::TokenType> oneOf);

    bool recoverUntil(TokenTreeIterator &it, const RecoverPredicate &predicate);

    static bool recoverUntil(TokenTreeIterator &it, const RecoverPredicate &predicate, errors::ErrorContext &errCtx);

    static lexer::Token consumeToken(TokenTreeIterator &it, lexer::TokenType type);

    static std::optional<lexer::Token> tryConsumeToken(TokenTreeIterator &it, lexer::TokenType type);

    const lexer::TokenTree &consumeTokenTree(TokenTreeIterator &it, lexer::TokenType type);

    std::optional<const lexer::TokenTree *> tryConsumeTokenTree(TokenTreeIterator &it, lexer::TokenType type);
};

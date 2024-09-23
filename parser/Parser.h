//
// Created by zoe on 07.09.24.
//


#pragma once

#include <functional>
#include <memory>

#include "errors/CompilerError.h"
#include "ast/FileUses.h"
#include "TokenTreeIterator.h"

class ImplSetter;
class ImplGetter;
class ImplMethod;
class DestructureDeclaration;
class ConstructorDeclaration;
class ErrorContext;
class Parameter;
class TupleSignature;
class FunctionSignature;
class TypeSignature;
class SignatureBase;
class InterfaceConstraint;
class ModuleDeclaration;
class Source;
class ReturnType;
class PropertyDeclaration;
class InterfaceSetter;
class InterfaceGetter;
class InterfaceMethodDeclaration;
class EnumMemberDeclaration;
class ConstraintDeclaration;
class ModuleRegistry;


class Parser {
    using RecoverPredicate = std::function<bool(const TokenTreeNode &)>;
public:
    std::shared_ptr<Source> source;

    explicit Parser(const std::shared_ptr<Source>& source);

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) noexcept;
    Parser& operator=(Parser&&) noexcept;
    ~Parser();

    std::vector<ModuleDeclaration> parse();

private:
    std::shared_ptr<FileUses> uses;
    std::vector<ModuleDeclaration> modules;

    void addError(CompilerError error);

    void parseFile();

    void useRule(TokenTreeIterator& it);

    void modRule(TokenTreeIterator& it);

    std::vector<Token> modifierRule(TokenTreeIterator& it, const RecoverPredicate& recoverPredicate);

    void validateModifiers(std::vector<Token> &modifiers, const std::vector<TokenType> &validTokenTypes);

    void enumRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    std::optional<EnumMemberDeclaration> enumMemberRule(TokenTreeIterator& it);

    void interfaceRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    InterfaceMethodDeclaration interfaceMethodRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    InterfaceGetter interfaceGetterRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    InterfaceSetter interfaceSetterRule(TokenTreeIterator& it, std::vector<Token> modifiers);


    void structRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    std::vector<ConstraintDeclaration> genericConstraintListRule(TokenTreeIterator& it, const RecoverPredicate &recoverPredicate);

    std::optional<PropertyDeclaration> propertyDeclarationRule(TokenTreeIterator& it);

    void functionRule(TokenTreeIterator& it, std::vector<Token> modifiers);

    void aliasRule(TokenTreeIterator& it, std::vector<Token> modifiers);

    void moduleVariableRule(TokenTreeIterator& it, std::vector<Token> modifiers);

    void declarationRule(TokenTreeIterator& it);

    void implRule(TokenTreeIterator& it);
    ConstructorDeclaration constructorRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    DestructureDeclaration destructorRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    ImplMethod methodRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    ImplGetter implGetterRule(TokenTreeIterator& it, std::vector<Token> modifiers);
    ImplSetter implSetterRule(TokenTreeIterator& it, std::vector<Token> modifiers);

    Path pathRule(TokenTreeIterator& it, bool allowTrailing);

    std::optional<ConstraintDeclaration> genericConstraintRule(TokenTreeIterator& it);

    std::optional<InterfaceConstraint> interfaceConstraintRule(TokenTreeIterator& it);
//    std::unique_ptr<GenericConstraintBase> defaultConstraintRule(TokenTreeIterator& it);

    [[nodiscard]] Identifier identifierRule(TokenTreeIterator& it) const;

    [[nodiscard]] std::unique_ptr<SignatureBase> signatureRule(TokenTreeIterator& it);
    [[nodiscard]] TypeSignature typeSignatureRule(TokenTreeIterator& it);
    [[nodiscard]] FunctionSignature functionSignatureRule(TokenTreeIterator& it);
    [[nodiscard]] TupleSignature tupleSignatureRule(TokenTreeIterator& it);

    [[nodiscard]] ReturnType returnTypeRule(TokenTreeIterator& it);

    [[nodiscard]] std::vector<Identifier> identifierListRule(const TokenTree &list);
    [[nodiscard]] std::vector<Parameter> parameterListRule(const TokenTree &list);
    [[nodiscard]] std::vector<std::unique_ptr<SignatureBase>> signatureListRule(const TokenTree &list);

    void recoverTopLevel(TokenTreeIterator& it);

    void recoverUntil(TokenTreeIterator& it, TokenType type);

    void recoverUntil(TokenTreeIterator& it, std::vector<TokenType> oneOf);

    void recoverUntil(TokenTreeIterator& it, const RecoverPredicate &predicate);

    static void recoverUntil(TokenTreeIterator& it, const RecoverPredicate &predicate, ErrorContext& errCtx);

    static Token consumeToken(TokenTreeIterator& it, TokenType type);
    static std::optional<Token> tryConsumeToken(TokenTreeIterator& it, TokenType type);

    const TokenTree& consumeTokenTree(TokenTreeIterator& it, TokenType type);
    std::optional<const TokenTree*> tryConsumeTokenTree(TokenTreeIterator& it, TokenType type);
};

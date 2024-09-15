//
// Created by zoe on 07.09.24.
//


#pragma once

#include <functional>
#include <memory>

#include "../errors/CompilerError.h"
#include "ast/FileUses.h"

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


using treeIterator = std::vector<TokenTreeNode>::const_iterator;

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

    void parse();

private:
    std::shared_ptr<FileUses> uses;
    std::vector<ModuleDeclaration> modules;

    void addError(CompilerError error);

    void parseFile();

    void useRule(treeIterator &start, const treeIterator &end);

    void modRule(treeIterator &start, const treeIterator &end);

    std::vector<Token> modifierRule(treeIterator &start, const treeIterator &end, const RecoverPredicate& recoverPredicate);

    void validateModifiers(std::vector<Token> &modifiers, const std::vector<TokenType> &validTokenTypes);

    void enumRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<EnumMemberDeclaration> enumMemberRule(treeIterator &start, const treeIterator &end);

    void interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<InterfaceMethodDeclaration> interfaceMethodRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<InterfaceGetter> interfaceGetterRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<InterfaceSetter> interfaceSetterRule(treeIterator &tart, const treeIterator &end, std::vector<Token> modifiers);


    void structRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::vector<ConstraintDeclaration> genericConstraintListRule(treeIterator &start, const treeIterator &end,
                                    const RecoverPredicate &recoverPredicate);

    std::optional<PropertyDeclaration> propertyDeclarationRule(treeIterator& start, const treeIterator& end);

    void functionRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void aliasRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void moduleVariableRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void declarationRule(treeIterator &start, const treeIterator &end);

    Path pathRule(treeIterator &start, const treeIterator &end, bool allowTrailing);

    std::optional<ConstraintDeclaration> genericConstraintRule(treeIterator &start, const treeIterator &end);

    std::optional<InterfaceConstraint> interfaceConstraintRule(treeIterator &start, const treeIterator &end);
//    std::unique_ptr<GenericConstraintBase> defaultConstraintRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] Identifier identifierRule(treeIterator &start, const treeIterator &end) const;

    [[nodiscard]] std::unique_ptr<SignatureBase> signatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] TypeSignature typeSignatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] FunctionSignature functionSignatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] TupleSignature tupleSignatureRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] ReturnType returnTypeRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] std::vector<Identifier> identifierListRule(const TokenTree &list);
    [[nodiscard]] std::vector<Parameter> parameterListRule(const TokenTree &list);
    [[nodiscard]] std::vector<std::unique_ptr<SignatureBase>> signatureListRule(const TokenTree &list);

    void recoverTopLevel(treeIterator &start, const treeIterator &end);

    void recoverUntil(treeIterator &start, const treeIterator &end, TokenType type);

    void recoverUntil(treeIterator &start, const treeIterator &end, std::vector<TokenType> oneOf);

    void recoverUntil(treeIterator &start, const treeIterator &end, const RecoverPredicate &predicate);

    static void recoverUntil(treeIterator &start, const treeIterator &end, const RecoverPredicate &predicate, ErrorContext& errCtx);

    static Token consumeToken(treeIterator &start, const treeIterator &end, TokenType type);
    static std::optional<Token> tryConsumeToken(treeIterator &start, const treeIterator &end, TokenType type);

    const TokenTree& consumeTokenTree(treeIterator &start, const treeIterator &end, TokenType type);
    std::optional<const TokenTree*> tryConsumeTokenTree(treeIterator &start, const treeIterator &end, TokenType type);
};

//
// Created by zoe on 07.09.24.
//


#pragma once

#include <functional>
#include <memory>

#include "../errors/CompilerError.h"
#include "ast/FileUses.h"

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

    void useRule(treeIterator &start, const treeIterator &end);

    void modRule(treeIterator &start, const treeIterator &end);

    std::vector<Token> modifierRule(treeIterator &start, const treeIterator &end);

    void validateModifiers(std::vector<Token> &modifiers, const std::vector<TokenType> &validTokenTypes);

    void enumRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<EnumMemberDeclaration> enumMemberRule(treeIterator &start, const treeIterator &end);

    std::optional<InterfaceMethodDeclaration> interfaceMethodRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<InterfaceGetter> interfaceGetterRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<InterfaceSetter> interfaceSetterRule(treeIterator &tart, const treeIterator &end, std::vector<Token> modifiers);

    void interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void structRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    std::optional<PropertyDeclaration> propertyDeclarationRule(treeIterator& start, const treeIterator& end);

    void functionRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void aliasRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void moduleVariableRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void declarationRule(treeIterator &start, const treeIterator &end);

    std::optional<Path> pathRule(treeIterator &start, const treeIterator &end, bool allowTrailing);

    ConstraintDeclaration genericConstraintRule(treeIterator &start, const treeIterator &end);

    std::optional<InterfaceConstraint> interfaceConstraintRule(treeIterator &start, const treeIterator &end);
//    std::unique_ptr<GenericConstraintBase> defaultConstraintRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] std::optional<Identifier> identifierRule(treeIterator &start, const treeIterator &end) const;

    [[nodiscard]] std::optional<std::unique_ptr<SignatureBase>> signatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] std::optional<TypeSignature> typeSignatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] std::optional<FunctionSignature> functionSignatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] std::optional<TupleSignature> tupleSignatureRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] std::optional<ReturnType> returnTypeRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] std::vector<Identifier> identifierListRule(const TokenTreeNode &node, TokenType opener);
    [[nodiscard]] std::vector<Parameter> parameterListRule(const TokenTreeNode &node, TokenType opener);
    [[nodiscard]] std::vector<std::unique_ptr<SignatureBase>> signatureListRule(const TokenTreeNode &node, TokenType opener);

    static void recoverTopLevel(treeIterator &start, const treeIterator &end);

    static void recoverUntil(treeIterator &start, const treeIterator &end, TokenType type, bool consume);

    static void recoverUntil(treeIterator &start, const treeIterator &end, std::vector<TokenType> oneOf, bool consume);

    static void recoverUntil(treeIterator &start, const treeIterator &end, const std::function<bool(const TokenTreeNode &)> &predicate, bool consume);
};

//
// Created by zoe on 07.09.24.
//


#pragma once

#include <functional>

#include "CompilerError.h"
#include "ast/FunctionSignature.h"
#include "ast/ModuleDeclaration.h"
#include "ast/TupleSignature.h"
#include "lexer/TokenTree.h"

using treeIterator = std::vector<TokenTreeNode>::const_iterator;

class Parser {
public:
    const std::shared_ptr<Source> source;

    explicit Parser(std::shared_ptr<Source> source)
            : source(std::move(source)) {
        modules.emplace_back();
        uses = std::make_shared<FileUses>();
    }

    std::vector<ModuleDeclaration> parse();

private:
    std::shared_ptr<FileUses> uses;
    std::vector<ModuleDeclaration> modules;

    void addError(const CompilerError &error);

    void parseFile();

    void useRule(treeIterator &start, const treeIterator &end);

    void modRule(treeIterator &start, const treeIterator &end);

    std::vector<Token> modifierRule(treeIterator &start, const treeIterator &end);

    void validateModifiers(std::vector<Token> &modifiers, const std::vector<TokenType> &validTokenTypes);

    void enumRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);
    std::optional<EnumMemberDeclaration> enumMemberRule(treeIterator &start, const treeIterator &end);

    void interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void structRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void functionRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void aliasRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void moduleVariableRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void declarationRule(treeIterator &start, const treeIterator &end);

    std::optional<Path> pathRule(treeIterator &start, const treeIterator &end, bool allowTrailing);

    std::optional<ConstraintDeclaration> genericConstraintRule(treeIterator &start, const treeIterator &end);

    std::unique_ptr<GenericConstraintBase> interfaceConstraintRule(treeIterator &start, const treeIterator &end);
//    std::unique_ptr<GenericConstraintBase> defaultConstraintRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] std::optional<Identifier> identifierRule(treeIterator &start, const treeIterator &end) const;

    [[nodiscard]] std::optional<std::unique_ptr<SignatureBase>> signatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] std::optional<std::unique_ptr<TypeSignature>> typeSignatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] std::optional<std::unique_ptr<FunctionSignature>> functionSignatureRule(treeIterator &start, const treeIterator &end);
    [[nodiscard]] std::optional<std::unique_ptr<TupleSignature>> tupleSignatureRule(treeIterator &start, const treeIterator &end);

    [[nodiscard]] std::vector<Identifier> identifierListRule(const TokenTreeNode &node, TokenType opener);
    [[nodiscard]] std::vector<Parameter> parameterListRule(const TokenTreeNode &node, TokenType opener);
    [[nodiscard]] std::vector<std::unique_ptr<SignatureBase>> signatureListRule(const TokenTreeNode &node, TokenType opener);

    static void recoverTopLevel(treeIterator &start, const treeIterator &end);

    static void recoverUntil(treeIterator &start, const treeIterator &end, TokenType type, bool consume);

    static void recoverUntil(treeIterator &start, const treeIterator &end, std::vector<TokenType> oneOf, bool consume);

    static void recoverUntil(treeIterator &start, const treeIterator &end, const std::function<bool(const TokenTreeNode &)> &predicate, bool consume);
};

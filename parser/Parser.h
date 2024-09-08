//
// Created by zoe on 07.09.24.
//


#pragma once

#include <functional>

#include "CompilerError.h"
#include "ast/ModuleDeclaration.h"
#include "lexer/TokenTree.h"

using treeIterator = std::vector<TokenTreeNode>::const_iterator;

class Parser {
public:
    const std::shared_ptr<Source> source;
    std::vector<CompilerError> errors;

    explicit Parser(std::shared_ptr<Source> source)
        : source(std::move(source)) {
        modules.emplace_back();
    }

    std::vector<ModuleDeclaration> parse();

private:

    std::vector<UseNode> useNodes;
    std::vector<ModuleDeclaration> modules;

    void addError(const CompilerError &error);

    void parseFile();

    void useRule(treeIterator &start, const treeIterator &end);

    void modRule(treeIterator &start, const treeIterator &end);

    std::vector<Token> modifierRule(treeIterator &start, const treeIterator &end);

    void enumRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void structRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void functionRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void aliasRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void moduleVariableRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers);

    void declarationRule(treeIterator &start, const treeIterator &end);

    std::optional<Path> pathRule(treeIterator &start, const treeIterator &end, bool allowTrailing);

    [[nodiscard]] std::optional<Identifier> identifierRule(treeIterator &start, const treeIterator &end) const;

    [[nodiscard]] std::vector<Identifier> identifierListRule(const TokenTreeNode &list, TokenType opener);

    static void recoverTopLevel(treeIterator &start, const treeIterator &end);
    static void recoverUntil(treeIterator &start, const treeIterator &end, TokenType type, bool consume);
    static void recoverUntil(treeIterator &start, const treeIterator &end, std::vector<TokenType> oneOf, bool consume);
    static void recoverUntil(treeIterator &start, const treeIterator &end, const std::function<bool(const TokenTreeNode&)> &predicate, bool consume);
};

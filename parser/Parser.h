//
// Created by zoe on 07.09.24.
//


#pragma once

#include "CompilerError.h"
#include "ast/ModuleDeclaration.h"
#include "lexer/TokenTree.h"

using treeIterator = std::vector<TokenTreeNode>::const_iterator;

class Parser {
public:
    const SourceMap &sources;
    std::vector<CompilerError> errors;

    Parser(const TokenTree &tokenTree, const SourceMap &sources)
        : tokenTree(tokenTree),
          sources(sources) {
        modules.emplace_back();
    }

    std::vector<ModuleDeclaration> parse();

private:
    const TokenTree &tokenTree;
    std::vector<UseNode> useNodes;
    std::vector<ModuleDeclaration> modules;

    void addError(const CompilerError &error);

    void parseFile();

    void useRule(treeIterator &start, const treeIterator &end);

    void modRule(treeIterator &start, const treeIterator &end);

    void declarationRule(treeIterator &start, const treeIterator &end);

    std::optional<Path> pathRule(treeIterator &start, const treeIterator &end, bool allowTrailing);

    [[nodiscard]] std::optional<Identifier> identifierRule(treeIterator &start, const treeIterator &end) const;

    static void recoverTopLevel(treeIterator &start, const treeIterator &end);
};

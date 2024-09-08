//
// Created by zoe on 07.09.24.
//

#include "Parser.h"

#include "InternalError.h"

std::vector<ModuleDeclaration> Parser::parse() {
    parseFile();

    //TODO: convert result

    return std::move(modules);
}

void Parser::addError(const CompilerError &error) {
    errors.push_back(error);
}

void Parser::parseFile() {
    COMPILER_ASSERT(tokenTree.left.type == TokenType::Bof, "Parse tree did not start with BOF.");
    COMPILER_ASSERT(tokenTree.right.isToken(TokenType::Eof), "Parse tree did not end with an EOF.");

    auto current = tokenTree.tokens.begin();
    const auto end = tokenTree.tokens.end();

    while (current != end) {
        const auto &tokenTree = *current;
        if (tokenTree.isTokenResult()) {
            const auto &tokenResult = tokenTree.getTokenResult();
            if (tokenResult.isError()) {
                //TODO: error
                continue;
            }
            auto& token = tokenResult.get();

            if (token.type == TokenType::Use) {
                useRule(current, end);
            } else if (token.type == TokenType::Mod) {
                modRule(current, end);
            } else if (token.type == TokenType::Pub || token.isDeclaratorKeyword()) {
                declarationRule(current, end);
            }
        }
    }
}

void Parser::useRule(treeIterator &start, const treeIterator &end) {
    auto current = start;
    current += 1;

    auto path = pathRule(start, end, true);
    if (!path) {
        auto error = CompilerError(UseIsMissingPath, (start)->getToken());
        error.addLabel("expected a module path here", (current)->getToken());
        addError(error);
        recoverTopLevel(start, end);
        return;
    }

    if(path->isTrailing()) {
        if(current == end || !current->isTokenTree(TokenType::OpenCurly)) {

        }
    }

    if(current == end || !current->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, (start)->getToken());
        addError(error);
    }
}

void Parser::modRule(treeIterator &start, const treeIterator &end) {
    start += 1;
}

void Parser::declarationRule(treeIterator &start, const treeIterator &end) {

}

std::optional<Path> Parser::pathRule(treeIterator &start, const treeIterator &end, bool allowTrailing) {
    if(start == end) {
        return std::nullopt;
    }

    Path path{};
    auto current = start;

    if(current->isToken(TokenType::PathSeparator)) {
        path.rooted = current->getToken();
        current += 1;
    }

    auto identifier = identifierRule(current, end);
    if(!identifier) {
        return std::nullopt;
    }
    path.parts.push_back(*identifier);

    while(current != end && current->isToken(TokenType::PathSeparator)) {
        current += 1;

        identifier = identifierRule(current, end);
        if(!identifier) {
            const auto separatorToken = (current-1)->getToken();

            if(!allowTrailing) {
                auto error = CompilerError(PathHasTrailingSeparator, separatorToken);
                error.addLabel("trailing path separator", separatorToken);
                addError(error);
                break;
            }

            path.trailer = separatorToken;
            break;
        }
        path.parts.push_back(*identifier);
    }

    start = current;
    return path;
}

std::optional<Identifier> Parser::identifierRule(treeIterator &start, const treeIterator &end) const {
    const auto current = start;
    if (!current->isToken(TokenType::Identifier)) {
        return std::nullopt;
    }

    start += 1;
    return Identifier(current->getToken(), sources);
}

void Parser::recoverTopLevel(treeIterator &start, const treeIterator &end) {
    while(start != end) {
        if(!start->isToken())
            continue;

        const auto current = start->getToken();
        if(current.type == TokenType::Semicolon) {
            start += 1;
            return;
        }

        if(current.isTopLevelStarter()) {
            return;
        }

        start += 1;
    }
}

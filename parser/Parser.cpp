//
// Created by zoe on 07.09.24.
//

#include "Parser.h"
#include "sourceMap/Source.h"
#include "InternalError.h"

std::vector<ModuleDeclaration> Parser::parse() {
    parseFile();

    //TODO: convert result

    return std::move(modules);
}

void Parser::addError(const CompilerError &error) {
    source->errors.push_back(error);
}

void Parser::parseFile() {
    COMPILER_ASSERT(source->tokenTree, "Token tree was not set: " + source->fileName);

    auto tokenTree = *source->tokenTree;
    COMPILER_ASSERT(tokenTree.left.type == TokenType::Bof, "Token tree did not start with BOF: " + source->fileName);
    COMPILER_ASSERT(tokenTree.right.isToken(TokenType::Eof), "Token tree did not end with an EOF: " + source->fileName);

    treeIterator current = tokenTree.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = tokenTree.tokens.end();

    while (current != end) {
        const auto &currentTokenTree = *current;
        if (currentTokenTree.isTokenResult()) {
            const auto &tokenResult = currentTokenTree.getTokenResult();
            if (tokenResult.isError()) {
                auto error = CompilerError(UnexpectedToken, current->getStart());
                error.addLabel("expected a top level declaration", *current);
                addError(error);
                recoverTopLevel(current, end);
                continue;
            }
            auto &token = tokenResult.get();

            if (token.type == TokenType::Use) {
                useRule(current, end);
            } else if (token.type == TokenType::Mod) {
                modRule(current, end);
            } else if (token.isModifier() || token.isDeclaratorKeyword()) {
                declarationRule(current, end);
            } else {
                auto error = CompilerError(UnexpectedToken, current->getStart());
                error.addLabel("expected a top level declaration", *current);
                addError(error);
                recoverTopLevel(current, end);
            }
        } else {
            auto error = CompilerError(UnexpectedToken, current->getStart());
            error.addLabel("expected a top level declaration", *current);
            addError(error);
            recoverTopLevel(current, end);
        }
    }
}

void Parser::useRule(treeIterator &start, const treeIterator &end) {
    if (modules.size() > 1) {
        auto error = CompilerError(UseAfterMod, start->getStart());
        error.setNote("uses need to be declared before any modules");
        addError(error);
    }

    COMPILER_ASSERT(start->isToken(TokenType::Use), "useRule called with non-use starting token");
    start += 1;

    auto use = UseNode();

    auto path = pathRule(start, end, true);
    if (!path) {
        auto error = CompilerError(UseIsMissingPath, start->getStart());
        error.addLabel("expected a module path here", *start);
        addError(error);
        recoverTopLevel(start, end);
        return;
    }

    use.path = *path;

    if (use.path.isTrailing()) {
        if (start == end || !start->isTokenTree(TokenType::OpenCurly)) {
            const auto separatorToken = (start - 1)->getToken();
            auto error = CompilerError(PathHasTrailingSeparator, separatorToken);
            error.addLabel("trailing path separator", separatorToken);
            addError(error);

            useNodes.push_back(use);
            return;
        }

        use.names = std::move(identifierListRule(*start, TokenType::OpenCurly));
    }

    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, start->getStart());
        addError(error);
    } else {
        start += 1;
    }

    useNodes.push_back(use);
}

void Parser::modRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start->isToken(TokenType::Mod), "modRule called with non-mod starting token");
    start += 1;

    auto path = pathRule(start, end, false);

    if (!path) {
        auto error = CompilerError(UseIsMissingPath, start->getStart());
        error.addLabel("expected a module path here", *start);
        addError(error);
        recoverTopLevel(start, end);
        return;
    }

    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, start->getStart());
        addError(error);
    } else {
        start += 1;
    }

    modules.emplace_back(*path);
}

std::vector<Token> Parser::modifierRule(treeIterator &start, const treeIterator &end) {
    std::vector<Token> result;
    auto startPosition = start->getStart();

    while(start->isToken()) {
        auto token = start->getToken();
        if(!token.isModifier()) {
            break;
        }

        auto alreadyContained = std::ranges::find_if(result.begin(), result.end(), [&token](auto &t) {
            return t.type == token.type;
        });

        if(alreadyContained != result.end()) {
            auto error = CompilerError(DuplicateModifier, startPosition);
            error.addLabel("duplicate use of", *start);
            error.addLabel("is already present here", *alreadyContained);
            error.setNote("a modifier is only allowed once per declaration");
            addError(error);
            recoverTopLevel(start, end);
        }else {
            result.emplace_back(start->getToken());
        }

        start += 1;
    }

    return result;
}

void Parser::enumRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {

}

void Parser::interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
}

void Parser::structRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
}

void Parser::functionRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
}

void Parser::aliasRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
}

void Parser::moduleVariableRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
}

void Parser::declarationRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start->isToken() && (start->isToken(TokenType::Pub) || start->getToken().isDeclaratorKeyword()),
                    "declarationRule called with non-declaration starting token");

    auto modifiers = modifierRule(start, end);

    const auto &currentTokenTree = *start;
    if (currentTokenTree.isTokenResult()) {
        const auto &tokenResult = currentTokenTree.getTokenResult();
        if (tokenResult.isError()) {
            auto error = CompilerError(UnexpectedToken, start->getStart());
            error.addLabel("expected a top level declaration", *start);
            addError(error);
            recoverTopLevel(start, end);
            return;
        }
        auto &token = tokenResult.get();

        if (token.type == TokenType::Enum) {
            enumRule(start, end, std::move(modifiers));
        } else if (token.type == TokenType::Interface) {
            interfaceRule(start, end,std::move(modifiers));
        } else if (token.type == TokenType::Struct) {
            structRule(start, end, std::move(modifiers));
        } else if (token.type == TokenType::Fn) {
            functionRule(start, end, std::move(modifiers));
        } else if (token.type == TokenType::Alias) {
            aliasRule(start, end, std::move(modifiers));
        } else if (token.type == TokenType::Let) {
            moduleVariableRule(start, end, std::move(modifiers));
        } else {
            auto error = CompilerError(UnexpectedToken, start->getStart());
            error.addLabel("expected a top level declaration", *start);
            addError(error);
            recoverTopLevel(start, end);
        }
    } else {
        auto error = CompilerError(UnexpectedToken, start->getStart());
        error.addLabel("expected a top level declaration", *start);
        addError(error);
        recoverTopLevel(start, end);
    }
}

std::optional<Path> Parser::pathRule(treeIterator &start, const treeIterator &end, bool allowTrailing) {
    if (start == end) {
        return std::nullopt;
    }

    Path path{};
    auto current = start;

    if (current->isToken(TokenType::PathSeparator)) {
        path.rooted = current->getToken();
        current += 1;
    }

    auto identifier = identifierRule(current, end);
    if (!identifier) {
        return std::nullopt;
    }
    path.parts.push_back(*identifier);

    while (current != end && current->isToken(TokenType::PathSeparator)) {
        current += 1;

        identifier = identifierRule(current, end);
        if (!identifier) {
            const auto separatorToken = (current - 1)->getToken();

            if (!allowTrailing) {
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
    return Identifier(current->getToken(), *source);
}

std::vector<Identifier> Parser::identifierListRule(const TokenTreeNode &node, TokenType opener) {
    if (!node.isTokenTree()) {
        auto error = CompilerError(UnexpectedToken, node.getStart());
        error.addLabel("unexpected token", node);
        error.setNote("expected a " + TokenTypeStringQuoted(opener));
        addError(error);
    }

    auto list = node.getTokenTree();

    std::vector<Identifier> result;

    if (list.left.type != opener) {
        auto error = CompilerError(WrongOpener, list.left);
        error.addLabel("wrong opener for list, expected: " + TokenTypeStringQuoted(opener), list.left);
        addError(error);
    } else if (list.right.isError()) {
        auto error = CompilerError(WrongCloser, list.left);
        error.addLabel(
            "wrong closer for list, expected: " + TokenTypeStringQuoted(list.left.expectedClosing()),
            list.right.getError().token);
        addError(error);
    }

    treeIterator current = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();

    while (current != end) {
        auto identifier = identifierRule(current, end);
        if (!identifier) {
            auto error = CompilerError(UnexpectedToken, list.left);
            error.addLabel("expected " + TokenTypeStringQuoted(TokenType::Identifier) + " in list", *current);
            addError(error);

            recoverUntil(current, end, TokenType::Comma, true);
            continue;
        }

        result.emplace_back(std::move(*identifier));

        if (current == end) {
            continue;
        }

        if (!current->isToken(TokenType::Comma)) {
            auto temp = current;
            auto nextIdentifier = identifierRule(current, end);
            if (nextIdentifier) {
                auto error = CompilerError(UnexpectedToken, list.left);
                error.addLabel("expected " + TokenTypeStringQuoted(TokenType::Identifier) + " in list", list.left);
                addError(error);
            }
            // ReSharper disable once CppDFAInvalidatedMemory
            current = temp;
            continue;
        }

        current += 1;
    }

    return result;
}

void Parser::recoverTopLevel(treeIterator &start, const treeIterator &end) {
    recoverUntil(start, end, [](const TokenTreeNode &node) {
        return node.isToken(TokenType::Semicolon)
               || (node.isToken() && node.getToken().isTopLevelStarter());
    }, false);

    if (start->isToken(TokenType::Semicolon)) {
        start += 1;
    }
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, TokenType type, bool consume) {
    recoverUntil(start, end, [type](const TokenTreeNode &node) {
        return node.isToken(type);
    }, consume);
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, std::vector<TokenType> oneOf, bool consume) {
    recoverUntil(start, end, [&oneOf](const TokenTreeNode &node) {
        return node.isToken() && std::ranges::find(oneOf, node.getToken().type) != oneOf.end();
    }, consume);
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end,
                          const std::function<bool(const TokenTreeNode &)> &predicate, bool consume) {
    while (start != end) {
        if (predicate(*start)) {
            if (consume) {
                start += 1;
            }
            return;
        }

        start += 1;
    }
}

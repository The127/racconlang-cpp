//
// Created by zoe on 07.09.24.
//

#include "Parser.h"
#include "sourceMap/Source.h"
#include "InternalError.h"

std::vector<ModuleDeclaration> Parser::parse() {
    parseFile();

    for (auto &module : modules) {
        module.uses = uses;
    }

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
    auto use = UseNode();
    use.startPos = start->getStart();
    start += 1;

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

            if (start == end) {
                use.endPos = (start - 1)->getEnd();
            } else {
                use.endPos = start->getEnd();
            }
            uses->uses.push_back(use);
            return;
        }

        use.names = std::move(identifierListRule(*start, TokenType::OpenCurly));
    }

    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, start->getStart());
        addError(error);

        use.endPos = (start - 1)->getEnd();
    } else {
        use.endPos = start->getEnd();
        start += 1;
    }

    uses->uses.push_back(use);
}

void Parser::modRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start->isToken(TokenType::Mod), "modRule called with non-mod starting token");

    auto mod = ModuleDeclaration();
    mod.startPos = start->getStart();

    start += 1;

    mod.path = pathRule(start, end, false);

    if (!mod.path) {
        auto error = CompilerError(UseIsMissingPath, start->getStart());
        error.addLabel("expected a module path here", *start);
        addError(error);
        recoverTopLevel(start, end);
        return;
    }

    mod.endPos = mod.path->end();
    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, start->getStart());
        addError(error);
    } else {
        mod.endPos = start->getEnd();
        start += 1;
    }

    modules.push_back(std::move(mod));
}

std::vector<Token> Parser::modifierRule(treeIterator &start, const treeIterator &end) {
    std::vector<Token> result;
    auto startPosition = start->getStart();

    while (start->isToken()) {
        auto token = start->getToken();
        if (!token.isModifier()) {
            break;
        }

        auto alreadyContained = std::ranges::find_if(result.begin(), result.end(), [&token](auto &t) {
            return t.type == token.type;
        });

        if (alreadyContained != result.end()) {
            auto error = CompilerError(DuplicateModifier, startPosition);
            error.addLabel("duplicate use of", *start);
            error.addLabel("is already present here", *alreadyContained);
            error.setNote("a modifier is only allowed once per declaration");
            addError(error);
            recoverTopLevel(start, end);
        } else {
            result.emplace_back(start->getToken());
        }

        start += 1;
    }

    return result;
}

void Parser::validateModifiers(std::vector<Token> &modifiers, const std::vector<TokenType> &validTokenTypes) {
    std::vector<Token> result;
    std::string note = "valid modifiers are: ";
    for (int i{}; i < validTokenTypes.size(); ++i) {
        note += TokenTypeStringQuoted(validTokenTypes[i]);
        if (i < validTokenTypes.size() - 2) {
            note += ", ";
        } else if (i < validTokenTypes.size() - 1) {
            note += " and ";
        }
    }

    for (auto modifier: modifiers) {
        auto isFound = std::ranges::find_if(validTokenTypes.begin(), validTokenTypes.end(), [&modifier](auto &t) {
            return modifier.type == t;
        }) == validTokenTypes.end();
        if (isFound) {
            result.emplace_back(modifier);
        } else {
            auto error = CompilerError(InvalidModifier, modifiers[0]);
            error.addLabel("not a valid modifier here", modifier);
            error.setNote(note);
            addError(error);
        }
    }

    modifiers = std::move(result);
}

inline bool containsModifier(const std::vector<Token> &modifiers, const TokenType type) {
    for (const auto modifier: modifiers) {
        if (modifier.type == type) {
            return true;
        }
    }
    return false;
}

void Parser::enumRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken() && start->isToken(TokenType::Enum),
                    "enumRule called with non-enum starting token");

    auto decl = EnumDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = start->getStart();
    }
    decl.endPos = start->getEnd();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected enum name", *start);
        addError(error);
    }
    decl.endPos = decl.name->end();

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    while (auto constraint = genericConstraintRule(start, end)) {
        decl.genericConstraints.emplace_back(std::move(*constraint));
        decl.endPos = constraint->end();
    }

    if(!start->isTokenTree(TokenType::OpenCurly)) {
        auto error = CompilerError(MissingEnumBody, decl.startPos);
        error.addLabel("expected start of enum body: `{`", *start);
        error.setNote("empty enums are not allowed");
        addError(error);
    }else {
        decl.endPos = start->getEnd();
        const auto &body = start->getTokenTree();

        if(body.right.isError()) {
            auto error = CompilerError(UnclosedEnumBody, body.left.start);
            error.addLabel("expected end of enum body: `}`", body.right);
            addError(error);
        }

        treeIterator bodyStart = body.tokens.begin(); // NOLINT(*-use-auto)
        const auto bodyEnd = body.tokens.end();

        while(auto member = enumMemberRule(bodyStart, bodyEnd)) {
            decl.memberDeclarations.emplace_back(std::move(*member));
        }
    }

    modules.back().enumDeclarations.emplace_back(std::move(decl));
}

std::optional<EnumMemberDeclaration> Parser::enumMemberRule(treeIterator &start, const treeIterator &end) {
    if(start == end) {
        return std::nullopt;
    }

    if(!start->isToken(TokenType::Identifier)) {
        auto error = CompilerError(EnumMemberExpected, start->getStart());
        error.addLabel("expected an enum member`}`", *start);
        addError(error);
        recoverUntil(start, end, TokenType::Identifier, false);
        return std::nullopt;
    }

    auto decl = EnumMemberDeclaration(*identifierRule(start, end));

    decl.startPos = decl.name.start();
    decl.endPos = decl.name.end();

    if(start == end) {
        return std::move(decl);
    }

    if(start->isTokenTree(TokenType::OpenParen)) {
        decl.endPos = start->getEnd();
        decl.values = signatureListRule(*start, TokenType::OpenParen);
    }

    if(start != end) {
        if(start->isToken(TokenType::Comma)) {
            start += 1;
        } else {
            auto error = CompilerError(MissingComma, start->getStart());
            error.addLabel("expected: `,``}`", *start);
            addError(error);
            recoverUntil(start, end, TokenType::Identifier, false);
        }
    }

    return std::move(decl);
}

void Parser::interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken() && start->isToken(TokenType::Interface),
                    "interfaceRule called with non-interface starting token");

    auto decl = InterfaceDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = start->getStart();
    }
    decl.endPos = start->getEnd();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected enum name", *start);
        addError(error);
    }
    decl.endPos = decl.name->end();

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    //TODO: require list

    while (auto constraint = genericConstraintRule(start, end)) {
        decl.genericConstraints.emplace_back(std::move(*constraint));
        decl.endPos = constraint->end();
    }
}

void Parser::structRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken() && start->isToken(TokenType::Struct),
                    "structRule called with non-struct starting token");

    auto decl = StructDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = start->getStart();
    }
    decl.endPos = start->getEnd();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected enum name", *start);
        addError(error);
    }
    decl.endPos = decl.name->end();

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    //TODO:
}

void Parser::functionRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken() && start->isToken(TokenType::Fn),
                    "functionRule called with non-fn starting token");

    auto decl = FunctionDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = start->getStart();
    }
    decl.endPos = start->getEnd();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected enum name", *start);
        addError(error);
    }
    decl.endPos = decl.name->end();

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    //TODO:
}

void Parser::aliasRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken() && start->isToken(TokenType::Alias),
                    "aliasRule called with non-alias starting token");

    auto decl = AliasDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = start->getStart();
    }
    decl.endPos = start->getEnd();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected enum name", *start);
        addError(error);
    }
    decl.endPos = decl.name->end();

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    //TODO:
}

void Parser::moduleVariableRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken() && start->isToken(TokenType::Let),
                    "moduleVariableRule called with non-let starting token");

    auto decl = ModuleVariableDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = start->getStart();
    }
    decl.endPos = start->getEnd();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected enum name", *start);
        addError(error);
    }
    decl.endPos = decl.name->end();

    //TODO:
}

void Parser::declarationRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start->isToken() && (start->getToken().isModifier() || start->getToken().isDeclaratorKeyword()),
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
            interfaceRule(start, end, std::move(modifiers));
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

std::optional<ConstraintDeclaration> Parser::genericConstraintRule(treeIterator &start, const treeIterator &end) {
    if (start == end) {
        return std::nullopt;
    }

    auto decl = ConstraintDeclaration();
    auto current = start;

    if (current->isToken(TokenType::Where)) {
        decl.startPos = current->getStart();
        current += 1;
    } else {
        return std::nullopt;
    }

    decl.name = std::move(identifierRule(current, end));
    if (!decl.name) {
        auto error = CompilerError(UnexpectedToken, decl.startPos);
        error.addLabel("expected generic parameter name", *current);
        addError(error);
    }

    if (!current->isToken(TokenType::Colon)) {
        auto error = CompilerError(UnexpectedToken, decl.startPos);
        error.addLabel("expected `:`", *current);
        addError(error);
    } else {
        current += 1;
    }

    while (current != end) {
        if (auto c1 = interfaceConstraintRule(current, end)) {
            decl.constraints.push_back(std::move(c1));
        } /* else if (auto c2 = defaultConstraintRule(current, end)) {
            decl.constraints.push_back(std::move(c2));
        }*/
        else if (current->isConstraintBreakout()) {
            break;
        } else {
            auto error = CompilerError(InvalidGenericConstraint, decl.startPos);
            error.addLabel("expected generic constraint", *current);
            addError(error);
            recoverUntil(current, end, [](const TokenTreeNode &node) {
                return node.isConstraintBreakout() || node.isToken(TokenType::Comma);
            }, false);
        }

        if (current != end && current->isToken(TokenType::Comma)) {
            current += 1;
        } else {
            break;
        }
    }


    decl.endPos = current->getEnd();
    start = current;
    return decl;
}

std::optional<Identifier> Parser::identifierRule(treeIterator &start, const treeIterator &end) const {
    const auto current = start;
    if (!current->isToken(TokenType::Identifier)) {
        return std::nullopt;
    }

    start += 1;
    return Identifier(current->getToken(), *source);
}

std::optional<std::unique_ptr<SignatureBase>> Parser::signatureRule(treeIterator &start, const treeIterator &end) {
    if(start == end) {
        return std::nullopt;
    }

    if(start->isToken(TokenType::Identifier) || start->isToken(TokenType::PathSeparator)) {
        auto type = typeSignatureRule(start, end);
        if(type) {
            COMPILER_ASSERT(*type, "empty unique pointer from typeSignatureRule");
            return std::move(*type);
        }
    }else if(start->isToken(TokenType::Fn)) {
        auto fn = functionSignatureRule(start, end);
        if(fn) {
            COMPILER_ASSERT(*fn, "empty unique pointer from functionSignatureRule");
            return std::move(*fn);
        }
    }else if(start->isTokenTree(TokenType::OpenParen)) {
        auto tuple = tupleSignatureRule(start, end);
        if(tuple) {
            COMPILER_ASSERT(*tuple, "empty unique pointer from tupleSignatureRule");
            return std::move(*tuple);
        }
    }

    return std::nullopt;
}

std::optional<std::unique_ptr<TypeSignature>> Parser::typeSignatureRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start == end || start->isToken(TokenType::Identifier), "typeSignatureRule called on non-identifier token tree node");

    auto decl = std::make_unique<TypeSignature>();
    decl->startPos = start->getStart();

    decl->path = std::move(*pathRule(start, end, false));
    decl->endPos = decl->path.end();

    if(start == end) {
        return std::move(decl);
    }

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl->endPos = start->getEnd();
        decl->genericArguments = std::move(signatureListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    return std::move(decl);
}

std::optional<std::unique_ptr<FunctionSignature>> Parser::functionSignatureRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start == end || start->isToken(TokenType::Fn), "functionSignatureRule called on non-fn token tree node");

    auto decl = std::make_unique<FunctionSignature>();
    decl->startPos = start->getStart();
    decl->endPos = start->getEnd();
    start += 1;

    if(start == end) {
        auto error = CompilerError(FnSignatureMissingParams, decl->start());
        error.addLabel("expected function parameters", decl->end(), decl->end());
        addError(error);
        return std::move(decl);
    }

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl->endPos = start->getEnd();
        decl->genericArguments = std::move(signatureListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    if(start == end) {
        auto error = CompilerError(FnSignatureMissingParams, decl->start());
        error.addLabel("expected function parameters", (start-1)->getEnd(), (start-1)->getEnd());
        addError(error);
        return std::move(decl);
    }

    if (!start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(FnSignatureMissingParams, decl->start());
        error.addLabel("expected function parameters", *start);
        addError(error);
    }else {
        decl->parameters = parameterListRule(*start, TokenType::OpenParen);
        start += 1;
    }

    decl->returnType = signatureRule(start, end);
    if(decl->returnType) {
        decl->endPos = (*decl->returnType)->end();
    }

    while (auto constraint = genericConstraintRule(start, end)) {
        decl->genericConstraints.emplace_back(std::move(*constraint));
        decl->endPos = constraint->end();
    }

    return std::move(decl);
}

std::optional<std::unique_ptr<TupleSignature>> Parser::tupleSignatureRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start == end || start->isTokenTree(TokenType::OpenParen), "tupleSignatureRule called on non-openParen token tree node");

    auto decl = std::make_unique<TupleSignature>();
    decl->startPos = start->getStart();
    decl->endPos = start->getEnd();

    decl->types = signatureListRule(*start, TokenType::OpenParen);
    start += 1;

    return std::move(decl);
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
            list.right.getError().got);
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

    return std::move(result);
}

std::vector<Parameter> Parser::parameterListRule(const TokenTreeNode &node, TokenType opener) {
    if (!node.isTokenTree()) {
        auto error = CompilerError(UnexpectedToken, node.getStart());
        error.addLabel("unexpected token", node);
        error.setNote("expected a " + TokenTypeStringQuoted(opener));
        addError(error);
    }

    auto list = node.getTokenTree();

    std::vector<Parameter> result;

    if (list.left.type != opener) {
        auto error = CompilerError(WrongOpener, list.left);
        error.addLabel("wrong opener for list, expected: " + TokenTypeStringQuoted(opener), list.left);
        addError(error);
    } else if (list.right.isError()) {
        auto error = CompilerError(WrongCloser, list.left);
        error.addLabel(
            "wrong closer for list, expected: " + TokenTypeStringQuoted(list.left.expectedClosing()),
            list.right.getError().got);
        addError(error);
    }

    treeIterator current = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();

    while (current != end) {
        auto identifier = identifierRule(current, end);
        if(!identifier) {
            auto error = CompilerError(ParameterNameMissing, node.getStart());
            error.addLabel("missing parameter name", *current);
            addError(error);
            recoverUntil(current, end, TokenType::Comma, true);
            continue;
        }

        auto signature = signatureRule(current, end);
        if (!signature) {
            auto error = CompilerError(ParameterTypeMissing, node.getStart());
            error.addLabel("missing parameter type", *current);
            addError(error);
            recoverUntil(current, end, TokenType::Comma, true);
            continue;
        }

        if(current != end) {
            if(node.isToken(TokenType::Comma)) {
                current += 1;
            }else {
                auto error = CompilerError(MissingComma, list.left);
                error.addLabel("expected a comma", *current);
                addError(error);
            }
        }
    }

    return std::move(result);
}

std::vector<std::unique_ptr<SignatureBase>> Parser::signatureListRule(const TokenTreeNode &node, TokenType opener) {
    if (!node.isTokenTree()) {
        auto error = CompilerError(UnexpectedToken, node.getStart());
        error.addLabel("unexpected token", node);
        error.setNote("expected a " + TokenTypeStringQuoted(opener));
        addError(error);
    }

    auto list = node.getTokenTree();

    std::vector<std::unique_ptr<SignatureBase>> result;

    if (list.left.type != opener) {
        auto error = CompilerError(WrongOpener, list.left);
        error.addLabel("wrong opener for list, expected: " + TokenTypeStringQuoted(opener), list.left);
        addError(error);
    } else if (list.right.isError()) {
        auto error = CompilerError(WrongCloser, list.left);
        error.addLabel(
            "wrong closer for list, expected: " + TokenTypeStringQuoted(list.left.expectedClosing()),
            list.right.getError().got);
        addError(error);
    }

    treeIterator current = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();

    while (current != end) {
        auto signature = signatureRule(current, end);
        if (signature) {
            result.emplace_back(std::move(*signature));
        } else {
            auto error = CompilerError(InvalidSignature, list.left);
            error.addLabel("expected type, tuple or function signature", *current);
            addError(error);
            recoverUntil(current, end, [](const TokenTreeNode &node) {
                return node.isSignatureStarter() || node.isToken(TokenType::Comma);
            }, false);
        }

        if(current != end) {
            if(current->isToken(TokenType::Comma)) {
              current += 1;
            } else {
                auto error = CompilerError(MissingComma, list.left);
                error.addLabel("expected a comma", *current);
                addError(error);
            }
        }
    }

    return std::move(result);
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

std::unique_ptr<GenericConstraintBase> Parser::interfaceConstraintRule(treeIterator &start, const treeIterator &end) {
    throw std::logic_error("not implemented");
}

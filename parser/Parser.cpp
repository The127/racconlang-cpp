//
// Created by zoe on 07.09.24.
//

#include "Parser.h"

#include <bits/ranges_algo.h>

#include <utility>

#include "sourceMap/Source.h"
#include "../errors/InternalError.h"
#include "ast/AliasDeclaration.h"
#include "ast/ConstraintDeclaration.h"
#include "ast/EnumDeclaration.h"
#include "ast/EnumMemberDeclaration.h"
#include "ast/FileUses.h"
#include "ast/FunctionDeclaration.h"
#include "ast/FunctionSignature.h"
#include "ast/InterfaceConstraint.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/InterfaceGetter.h"
#include "ast/InterfaceMethodDeclaration.h"
#include "ast/InterfaceSetter.h"
#include "ast/ModuleDeclaration.h"
#include "ast/ModuleVariableDeclaration.h"
#include "ast/PropertyDeclaration.h"
#include "ast/StructDeclaration.h"
#include "ast/TupleSignature.h"
#include "ast/UseNode.h"

Parser::Parser(const std::shared_ptr<Source>& source)
    : source(source) {
    modules.emplace_back();
    uses = std::make_shared<FileUses>();
}

Parser::Parser(Parser &&) noexcept = default;
Parser &Parser::operator=(Parser &&) noexcept = default;

Parser::~Parser() = default;

std::vector<ModuleDeclaration> Parser::parse() {
    parseFile();

    for (auto &module: modules) {
        module.uses = uses;
    }

    return std::move(modules);
}

void Parser::addError(CompilerError error) {
    source->addError(std::move(error));
}

void Parser::parseFile() {
    COMPILER_ASSERT(source->tokenTree, "Token tree was not set: " + source->fileName);

    const auto &tokenTree = *source->tokenTree;
    COMPILER_ASSERT(tokenTree.left.type == TokenType::Bof, "Token tree did not start with BOF: " + source->fileName);
    COMPILER_ASSERT(tokenTree.right.isToken(TokenType::Eof), "Token tree did not end with an EOF: " + source->fileName);

    treeIterator current = tokenTree.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = tokenTree.tokens.end();

    while (current != end) {
        if (current->isToken(TokenType::Use)) {
            useRule(current, end);
        } else if (current->isToken(TokenType::Mod)) {
            modRule(current, end);
        } else if (current->isModifier() || current->isDeclaratorKeyword()) {
            declarationRule(current, end);
        } else {
            auto error = CompilerError(UnexpectedToken, current->getStart());
            error.addLabel("expected a top level declaration", *current);
            addError(std::move(error));
            recoverTopLevel(current, end);
        }
    }
}

void Parser::useRule(treeIterator &start, const treeIterator &end) {
    if (modules.size() > 1) {
        auto error = CompilerError(UseAfterMod, start->getStart());
        error.setNote("uses need to be declared before any modules");
        addError(std::move(error));
    }

    COMPILER_ASSERT(start->isToken(TokenType::Use), "useRule called with non-use starting token");
    auto use = UseNode();
    use.startPos = start->getStart();
    start += 1;

    auto path = pathRule(start, end, true);
    if (!path) {
        auto error = CompilerError(UseIsMissingPath, start->getStart());
        error.addLabel("expected a module path here", *start);
        addError(std::move(error));
        recoverTopLevel(start, end);
        return;
    }

    use.path = std::move(*path);

    if (use.path.isTrailing()) {
        if (start == end || !start->isTokenTree(TokenType::OpenCurly)) {
            const auto separatorToken = (start - 1)->getToken();
            auto error = CompilerError(PathHasTrailingSeparator, separatorToken);
            error.addLabel("trailing path separator", separatorToken);
            addError(std::move(error));

            if (start == end) {
                use.endPos = (start - 1)->getEnd();
            } else {
                use.endPos = start->getEnd();
            }
            uses->uses.emplace_back(std::move(use));
            return;
        }

        use.names = std::move(identifierListRule(*start, TokenType::OpenCurly));
    }

    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, start->getStart());
        addError(std::move(error));

        use.endPos = (start - 1)->getEnd();
    } else {
        use.endPos = start->getEnd();
        start += 1;
    }

    uses->uses.emplace_back(std::move(use));
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
        addError(std::move(error));
        recoverTopLevel(start, end);
        return;
    }

    mod.endPos = mod.path->end();
    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, start->getStart());
        addError(std::move(error));
    } else {
        mod.endPos = start->getEnd();
        start += 1;
    }

    modules.push_back(std::move(mod));
}

std::vector<Token> Parser::modifierRule(treeIterator &start, const treeIterator &end) {
    std::vector<Token> result;
    auto startPosition = start->getStart();

    while (start != end && start->isModifier()) {
        const auto &token = start->getToken();

        auto alreadyContained = std::ranges::find_if(result.begin(), result.end(), [&token](auto &t) {
            return t.type == token.type;
        });

        if (alreadyContained != result.end()) {
            auto error = CompilerError(DuplicateModifier, startPosition);
            error.addLabel("duplicate use of", *start);
            error.addLabel("is already present here", *alreadyContained);
            error.setNote("a modifier is only allowed once per declaration");
            addError(std::move(error));
            recoverTopLevel(start, end);
        } else {
            result.emplace_back(start->getToken());
        }

        start += 1;
    }

    return result;
}

void Parser::validateModifiers(std::vector<Token> &modifiers, const std::vector<TokenType> &validTokenTypes) {
    // TODO: check that the order is correct (but still return all the valid modifiers)
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
        }) != validTokenTypes.end();
        if (isFound) {
            result.emplace_back(modifier);
        } else {
            auto error = CompilerError(InvalidModifier, modifiers[0]);
            error.addLabel("not a valid modifier here", modifier);
            error.setNote(note);
            addError(std::move(error));
        }
    }

    modifiers = std::move(result);
}

inline bool containsModifier(const std::vector<Token> &modifiers, const TokenType type) {
    return std::ranges::any_of(modifiers, [type](auto &m) { return m.type == type; });
}

void Parser::enumRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken(TokenType::Enum),
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

    if (start == end) {
        auto error = CompilerError(UnexpectedEndOfInput, decl.startPos);
        addError(std::move(error));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected enum name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(MissingEnumBody, decl.startPos);
        addError(std::move(error));
        modules.back().enumDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    while (start != end && start->isToken(TokenType::Where)) {
        auto &constraint = decl.genericConstraints.emplace_back(genericConstraintRule(start, end));
        decl.endPos = constraint.end();

        auto beforeRecover = start;
        if (start == end) break;
        recoverUntil(start, end, [](auto &node) {
            return node.isToken(TokenType::Where)
                   || node.isTokenTree(TokenType::OpenCurly)
                   || node.isTopLevelStarter();
        }, false);
        if (beforeRecover != start) {
            auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
    }

    if (start == end) {
        auto error = CompilerError(MissingEnumBody, decl.startPos);
        addError(std::move(error));
        modules.back().enumDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!start->isTokenTree(TokenType::OpenCurly)) {
        auto error = CompilerError(MissingEnumBody, decl.startPos);
        error.addLabel("expected start of enum body: `{`", *start);
        error.setNote("empty enums are not allowed");
        addError(std::move(error));
    } else {
        decl.endPos = start->getEnd();
        const auto &body = start->getTokenTree();

        if (body.right.isError()) {
            auto error = CompilerError(UnclosedEnumBody, body.left.start);
            error.addLabel("expected end of enum body: `}`", body.right);
            addError(std::move(error));
        }

        treeIterator bodyStart = body.tokens.begin(); // NOLINT(*-use-auto)
        const auto bodyEnd = body.tokens.end();

        while (auto member = enumMemberRule(bodyStart, bodyEnd)) {
            decl.memberDeclarations.emplace_back(std::move(*member));
        }
        start += 1;
    }

    modules.back().enumDeclarations.emplace_back(std::move(decl));
}

std::optional<EnumMemberDeclaration> Parser::enumMemberRule(treeIterator &start, const treeIterator &end) {
    if (start == end) {
        return std::nullopt;
    }

    if (!start->isToken(TokenType::Identifier)) {
        auto error = CompilerError(EnumMemberExpected, start->getStart());
        error.addLabel("expected an enum member`}`", *start);
        addError(std::move(error));
        recoverUntil(start, end, TokenType::Identifier, false);
        return std::nullopt;
    }

    auto decl = EnumMemberDeclaration(*identifierRule(start, end));

    decl.startPos = decl.name.start();
    decl.endPos = decl.name.end();

    if (start == end) {
        return std::move(decl);
    }

    if (start->isTokenTree(TokenType::OpenParen)) {
        decl.endPos = start->getEnd();
        decl.values = signatureListRule(*start, TokenType::OpenParen);
        start += 1;
    }

    if (start != end) {
        if (start->isToken(TokenType::Comma)) {
            start += 1;
        } else {
            auto error = CompilerError(MissingComma, start->getStart());
            error.addLabel("expected: `,` or `}`", *start);
            addError(std::move(error));
            recoverUntil(start, end, TokenType::Identifier, false);
        }
    }

    return std::move(decl);
}

std::optional<InterfaceMethodDeclaration> Parser::interfaceMethodRule(treeIterator &start, const treeIterator &end,
                                                                      std::vector<Token> modifiers) {
    if (start == end) {
        return std::nullopt;
    }
    COMPILER_ASSERT(start->isToken(TokenType::Fn),
                    "interfaceMethodRule called with non-fn starting token");

    auto decl = InterfaceMethodDeclaration();
    decl.startPos = start->getStart();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});
    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
        decl.isMut = containsModifier(modifiers, TokenType::Mut);
    }

    if (start == end) {
        auto error = CompilerError(MissingMethodName, (start - 1)->getStart());
        error.setNote("unexpected end of method declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingMethodName, decl.startPos);
        error.addLabel("expected method name", *start);
        addError(std::move(error));
        return std::nullopt;
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(UnexpectedToken, (start - 1)->getStart());
        error.setNote("unexpected end of method declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    if (!start->isTokenTree(TokenType::OpenAngle) && !start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(UnexpectedToken, decl.start());
        error.addLabel("expected generic parameters or method parameters", *start);
        addError(std::move(error));
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTokenTree(TokenType::OpenParen)
                   || node.isTokenTree(TokenType::OpenAngle)
                   || node.isToken(TokenType::Semicolon);
        }, false);
        if (start->isToken(TokenType::Semicolon)) {
            start += 1;
            return std::nullopt;
        }
        if (start == end) {
            return std::nullopt;
        }
    }

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    if (start == end) {
        auto error = CompilerError(UnexpectedToken, (start - 1)->getStart());
        error.setNote("unexpected end of method declaration, expected method parameters");
        addError(std::move(error));
        return std::nullopt;
    }

    if (!start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(UnexpectedToken, decl.start());
        error.addLabel("expected method parameters", *start);
        addError(std::move(error));
        return std::nullopt;
    }

    decl.parameters = parameterListRule(*start, TokenType::OpenParen);
    start += 1;

    if (start == end) {
        auto error = CompilerError(UnexpectedToken, (start - 1)->getStart());
        error.setNote("unexpected end of method declaration, expected return type, generic constraints or `;`");
        addError(std::move(error));
        return std::make_optional(std::move(decl));
    }

    if (start->isToken(TokenType::DashArrow)) {
        start += 1;
        if (start == end) {
            auto error = CompilerError(UnexpectedToken, (start - 1)->getStart());
            error.setNote("unexpected end of method declaration, expected return type, generic constraints or `;`");
            addError(std::move(error));
            return std::nullopt;
        }

        decl.returnType = returnTypeRule(start, end);
        if (!decl.returnType) {
            auto error = CompilerError(MissingMethodReturnType, decl.start());
            error.addLabel("expected method return type", *start);
            addError(std::move(error));
            return std::nullopt;
        }
    }

    while (start != end && start->isToken(TokenType::Where)) {
        auto &constraint = decl.genericConstraints.emplace_back(genericConstraintRule(start, end));
        decl.endPos = constraint.end();

        auto beforeRecover = start;
        if (start == end) break;
        recoverUntil(start, end, [](auto &node) {
            return node.isToken(TokenType::Where)
                   || node.isToken(TokenType::Semicolon)
                   || node.isTopLevelStarter();
        }, false);
        if (beforeRecover != start) {
            auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
    }

    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, (start - 1)->getStart());
        addError(std::move(error));
        return std::move(decl);
    }

    decl.endPos = start->getEnd();
    start += 1;

    return std::move(decl);
}

std::optional<InterfaceGetter> Parser::interfaceGetterRule(treeIterator &start, const treeIterator &end,
                                                           std::vector<Token> modifiers) {
    if (start == end) {
        return std::nullopt;
    }
    COMPILER_ASSERT(start->isToken(TokenType::Get),
                    "interfaceGetterRule called with non-get starting token");

    auto decl = InterfaceGetter();
    decl.startPos = start->getStart();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});
    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
        decl.isMut = containsModifier(modifiers, TokenType::Mut);
    }

    if (start == end) {
        auto error = CompilerError(MissingGetterName, (start - 1)->getStart());
        error.setNote("unexpected end of getter declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingGetterName, decl.startPos);
        error.addLabel("expected getter name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end || !start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(MissingSetterParam, (start - 1)->getStart());
        error.setNote("unexpected end of setter declaration, expected `(`");
        addError(std::move(error));
        return std::move(decl);
    }

    decl.endPos = start->getEnd();
    auto params = parameterListRule(*start, TokenType::OpenParen);

    if (!params.empty()) {
        auto error = CompilerError(TooManySetterParams, params[1].start());
        error.setNote("getters cannot have parameters");
        addError(std::move(error));
    }

    if (start == end) {
        auto error = CompilerError(MissingGetterReturnType, (start - 1)->getStart());
        error.setNote("unexpected end of getter declaration, expected return type`->`");
        addError(std::move(error));
        return std::move(decl);
    }

    if (!start->isToken(TokenType::DashArrow)) {
        auto error = CompilerError(MissingGetterReturnType, decl.startPos);
        error.addLabel("unexpected end of getter declaration, expected return type: `->`", *start);
        addError(std::move(error));
        return std::move(decl);
    }

    decl.endPos = start->getEnd();
    start += 1;

    decl.returnType = returnTypeRule(start, end);
    if (!decl.returnType) {
        auto error = CompilerError(MissingGetterReturnType, decl.startPos);
        error.addLabel("unexpected end of getter declaration, expected return type", *start);
        addError(std::move(error));
        return std::move(decl);
    }

    if (start == end) {
        auto error = CompilerError(MissingSemicolon, (start - 1)->getStart());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    if (!start->isToken(TokenType::Semicolon)) {
        if (start == end) {
            auto error = CompilerError(MissingSemicolon, start->getStart());
            error.setNote("expected a semicolon");
            addError(std::move(error));
        }
    } else {
        decl.endPos = start->getEnd();
        start += 1;
    }

    return std::move(decl);
}

std::optional<InterfaceSetter> Parser::interfaceSetterRule(treeIterator &start, const treeIterator &end,
                                                           std::vector<Token> modifiers) {
    if (start == end) {
        return std::nullopt;
    }
    COMPILER_ASSERT(start->isToken(TokenType::Set),
                    "interfaceSetterRule called with non-set starting token");

    auto decl = InterfaceSetter();
    decl.startPos = start->getStart();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
    }

    if (start == end) {
        auto error = CompilerError(MissingSetterName, (start - 1)->getStart());
        error.setNote("unexpected end of setter declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingSetterName, decl.startPos);
        error.addLabel("expected setter name", *start);
        addError(std::move(error));
        return std::nullopt;
    }
    decl.endPos = decl.name->end();

    if (start == end || !start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(MissingSetterParam, (start - 1)->getStart());
        error.setNote("unexpected end of setter declaration, expected `(`");
        addError(std::move(error));
        return std::move(decl);
    }

    decl.endPos = start->getEnd();
    auto params = parameterListRule(*start, TokenType::OpenParen);

    if (params.size() > 1) {
        auto error = CompilerError(TooManySetterParams, params[1].start());
        error.setNote("setters must have exactly 1 parameter");
        addError(std::move(error));
    }

    if (params.empty()) {
        auto error = CompilerError(MissingSetterParam, (start - 1)->getStart());
        error.setNote("setters must have exactly 1 parameter");
        addError(std::move(error));
    } else {
        decl.parameter = std::move(params[0]);
    }

    if (start == end) {
        auto error = CompilerError(MissingSemicolon, (start - 1)->getStart());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    if (!start->isToken(TokenType::Semicolon)) {
        if (start == end) {
            auto error = CompilerError(MissingSemicolon, start->getStart());
            error.setNote("expected a semicolon");
            addError(std::move(error));
        }
    } else {
        decl.endPos = start->getEnd();
        start += 1;
    }

    return std::move(decl);
}

void Parser::interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken(TokenType::Interface),
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
        error.addLabel("expected interface name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(MissingSemicolon, decl.startPos);
        error.addLabel("expected semicolon", *start);
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    if (start == end) {
        auto error = CompilerError(MissingSemicolon, decl.startPos);
        error.addLabel("expected semicolon", *start);
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!start->isToken(TokenType::Colon)
        && !start->isToken(TokenType::Where)
        && !start->isTokenTree(TokenType::OpenCurly)) {
        auto error = CompilerError(UnexpectedToken, decl.startPos);
        error.addLabel(
            "unexpected token, expected: inheritance list (`:`), constraints (`where`), interface body (`{`)",
            *start);
        addError(std::move(error));

        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::Colon)
                   || node.isToken(TokenType::Where)
                   || node.isTokenTree(TokenType::OpenCurly);
        }, false);
    }

    if (start == end) {
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start->isTopLevelStarter()) {
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start->isToken(TokenType::Colon)) {
        decl.endPos = start->getEnd();
        start += 1;

        while (start != end) {
            auto signature = typeSignatureRule(start, end);

            if (!signature) {
                auto error = CompilerError(UnexpectedToken, decl.startPos);
                error.addLabel("expected type signature in inheritance list", *start);
                addError(std::move(error));

                recoverUntil(start, end, [](const TokenTreeNode &node) {
                    return node.isTopLevelStarter()
                           || node.isToken(TokenType::Comma)
                           || node.isToken(TokenType::Where)
                           || node.isTokenTree(TokenType::OpenCurly);
                }, false);
                if (start != end && start->isToken(TokenType::Comma)) {
                    decl.endPos = start->getEnd();
                    start += 1;
                    continue;
                }
                if (start == end || start->isTopLevelStarter()) {
                    addError(CompilerError(MissingInterfaceBody, (start-1)->getStart()));
                    modules.back().interfaceDeclarations.emplace_back(std::move(decl));
                    return;
                }
                break;
            }

            decl.requiredInterfaces.emplace_back(std::move(*signature));

            if (start == end) {
                addError(CompilerError(MissingInterfaceBody, (start-1)->getStart()));
                modules.back().interfaceDeclarations.emplace_back(std::move(decl));
                return;
            }
            if (start->isToken(TokenType::Comma)) {
                decl.endPos = start->getEnd();
                start += 1;
            } else if (start->isSignatureStarter())  {
                auto error = CompilerError(MissingComma, decl.startPos);
                error.addLabel("missing comma", *start);
                addError(std::move(error));
            } else {
                break;
            }
        }
    }

    if (start == end) {
        auto error = CompilerError(MissingInterfaceBody, decl.startPos);
        error.addLabel(
            "missing interface body",
            *start);
        addError(std::move(error));
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!start->isToken(TokenType::Where)
        && !start->isTokenTree(TokenType::OpenCurly)) {
        auto error = CompilerError(UnexpectedToken, decl.startPos);
        error.addLabel(
            "unexpected token, expected: constraints (`where`), interface body (`{`)",
            *start);
        addError(std::move(error));

        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::Where)
                   || node.isTokenTree(TokenType::OpenCurly);
        }, false);

        if (start == end) {
            modules.back().interfaceDeclarations.emplace_back(std::move(decl));
            return;
        }
    }


    if (start->isTopLevelStarter()) {
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    while (start != end && start->isToken(TokenType::Where)) {
        auto &constraint = decl.genericConstraints.emplace_back(genericConstraintRule(start, end));
        decl.endPos = constraint.end();

        auto beforeRecover = start;
        if (start == end) break;
        recoverUntil(start, end, [](auto &node) {
            return node.isToken(TokenType::Where)
                   || node.isTokenTree(TokenType::OpenCurly)
                   || node.isTopLevelStarter();
        }, false);
        if (beforeRecover != start) {
            auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
    }

    if (start == end || !start->isTokenTree(TokenType::OpenCurly)) {
        auto error = CompilerError(MissingInterfaceBody, decl.startPos);
        error.addLabel("missing interface body", *(start - 1));
        addError(std::move(error));
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.endPos = start->getEnd();

    const auto &tokenTree = start->getTokenTree();
    if (tokenTree.right.isError()) {
        auto error = CompilerError(WrongCloser, tokenTree.left);
        error.addLabel(
            "wrong closer for interface body, expected: " + TokenTypeStringQuoted(tokenTree.left.expectedClosing()),
            tokenTree.right.getError().got);
        addError(std::move(error));
    }

    treeIterator bodyStart = tokenTree.tokens.begin(); // NOLINT(*-use-auto)
    auto bodyEnd = tokenTree.tokens.end();

    while (bodyStart != bodyEnd) {
        auto originalStart = bodyStart;
        std::vector<Token> memberModifiers;
        if (bodyStart->isModifier()) {
            memberModifiers = modifierRule(bodyStart, bodyEnd);
            validateModifiers(memberModifiers, {TokenType::Pub, TokenType::Mut});
        }
        if (bodyStart == bodyEnd) {
            auto error = CompilerError(UnexpectedToken, originalStart->getStart());
            error.addLabel("unexpected token here", *originalStart);
            break;
        }

        if (bodyStart->isToken(TokenType::Fn)) {
            auto method = interfaceMethodRule(bodyStart, bodyEnd, std::move(memberModifiers));
            if (method) {
                decl.methods.emplace_back(std::move(*method));
            }
        } else if (bodyStart->isToken(TokenType::Get)) {
            auto getter = interfaceGetterRule(bodyStart, bodyEnd, std::move(memberModifiers));
            if (getter) {
                decl.getters.emplace_back(std::move(*getter));
            }
        } else if (bodyStart->isToken(TokenType::Set)) {
            auto setter = interfaceSetterRule(bodyStart, bodyEnd, std::move(memberModifiers));
            if (setter) {
                decl.setters.emplace_back(std::move(*setter));
            }
        } else {
            auto error = CompilerError(UnexpectedToken, originalStart->getStart());
            error.addLabel("unexpected token here", *bodyStart);
            recoverUntil(bodyStart, bodyEnd, [](const TokenTreeNode &node) {
                return node.isModifier()
                       || node.isToken(TokenType::Get)
                       || node.isToken(TokenType::Set);
            }, false);
        }
    }


    modules.back().interfaceDeclarations.emplace_back(std::move(decl));
}

void Parser::structRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken(TokenType::Struct),
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

    if (start == end) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected struct name", *start);
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected struct name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(MissingSemicolon, decl.startPos);
        error.addLabel("expected semicolon", *start);
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericParams = std::move(identifierListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    if (start == end) {
        auto error = CompilerError(MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *start);
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    auto beforeRecover = start;
    recoverUntil(start, end, [](const TokenTreeNode &node) {
        return node.isTokenTree(TokenType::OpenCurly)
               || node.isTokenTree(TokenType::OpenParen)
               || node.isToken(TokenType::Where)
               || node.isTopLevelStarter();
    }, false);
    if (beforeRecover != start) {
        auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end || start->isTopLevelStarter()) {
        auto error = CompilerError(MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *(start - 1));
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    while (start != end && start->isToken(TokenType::Where)) {
        const auto &constraint = decl.genericConstraints.emplace_back(std::move(genericConstraintRule(start, end)));
        decl.endPos = constraint.end();
        beforeRecover = start;
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isToken(TokenType::Where)
                   || node.isTokenTree(TokenType::OpenCurly)
                   || node.isTokenTree(TokenType::OpenParen)
                   || node.isTopLevelStarter();
        }, false);
        if (beforeRecover != start) {
            auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
    }

    if (start == end || start->isTopLevelStarter()) {
        auto error = CompilerError(MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *(start - 1));
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start->isTokenTree(TokenType::OpenParen)) {
        auto parameters = parameterListRule(*start, TokenType::OpenParen);
        for (auto &parameter: parameters) {
            if (parameter.isMut || parameter.isRef) {
                auto error = CompilerError(InvalidModifier, parameter.startPos);
                error.setNote("short body struct properties cannot be `mut` and are `pub` by default");
                addError(std::move(error));
            }

            auto prop = PropertyDeclaration(parameter.name);
            prop.type = std::move(parameter.type);
            prop.isPublic = true;
            prop.startPos = parameter.start();
            prop.endPos = parameter.end();

            decl.propertyDeclarations.push_back(std::move(prop));

            decl.destructureProperties.push_back(parameter.name);
        }
        decl.endPos = start->getEnd();
        start += 1;

        beforeRecover = start;
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::Semicolon)
                   || node.isToken(TokenType::Where);
        }, false);
        if (beforeRecover != start) {
            auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
        if (start == end || start->isTopLevelStarter()) {
            auto error = CompilerError(MissingSemicolon, (start - 1)->getStart());
            addError(std::move(error));
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }

        while (start != end && start->isToken(TokenType::Where)) {
            const auto &constraint = decl.genericConstraints.emplace_back(std::move(genericConstraintRule(start, end)));
            decl.endPos = constraint.end();
            beforeRecover = start;
            recoverUntil(start, end, [](const TokenTreeNode &node) {
                return node.isToken(TokenType::Where)
                       || node.isToken(TokenType::Semicolon)
                       || node.isTopLevelStarter();
            }, false);
            if (beforeRecover != start) {
                auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
                addError(std::move(error));
            }
        }
        if (start == end || !start->isToken(TokenType::Semicolon)) {
            auto error = CompilerError(MissingSemicolon, (start - 1)->getStart());
            addError(std::move(error));
        }
    } else if (start->isTokenTree(TokenType::OpenCurly)) {
        const auto &body = start->getTokenTree();
        treeIterator bodyStart = body.tokens.begin(); // NOLINT(*-use-auto)
        auto bodyEnd = body.tokens.end();

        while (bodyStart != bodyEnd) {
            auto prop = propertyDeclarationRule(bodyStart, bodyEnd);
            if (prop) {
                decl.propertyDeclarations.emplace_back(std::move(*prop));
            }
        }

        start += 1;
        beforeRecover = start;
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::DestructuresInto);
        }, false);
        if (start != end && start->isToken(TokenType::DestructuresInto)) {
            decl.endPos = start->getEnd();
            start += 1;
            if (start == end) {
                auto error = CompilerError(UnexpectedEndOfInput, (start - 1)->getStart());
                addError(std::move(error));
                modules.back().structDeclarations.emplace_back(std::move(decl));
                return;
            }
            decl.destructureProperties = identifierListRule(*start, TokenType::OpenParen);

            beforeRecover = start;
            recoverUntil(start, end, [](const TokenTreeNode &node) {
                return node.isTopLevelStarter()
                       || node.isToken(TokenType::Semicolon);
            }, false);

            if (beforeRecover != start) {
                auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
                addError(std::move(error));
            }
            if (start == end || !start->isToken(TokenType::Semicolon)) {
                auto error = CompilerError(MissingSemicolon, beforeRecover->getStart());
                addError(std::move(error));
            }
            start += 1;
        }
    } else {
        COMPILER_ASSERT(false, "unreachable");
    }

    modules.back().structDeclarations.emplace_back(std::move(decl));
}


std::optional<PropertyDeclaration> Parser::propertyDeclarationRule(treeIterator &start, const treeIterator &end) {
    auto beforeRecover = start;
    recoverUntil(start, end, [](const TokenTreeNode &node) {
        return node.isModifier() || node.isToken(TokenType::Identifier) || node.isToken(TokenType::Semicolon);
    }, false);
    if (beforeRecover != start) {
        auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end) {
        //BEAN
        return std::nullopt;
    }
    if (start->isToken(TokenType::Semicolon)) {
        if (beforeRecover == start) {
            auto error = CompilerError(UnexpectedToken, start->getStart());
            addError(std::move(error));
        }
        start += 1;
        return std::nullopt;
    }

    auto modifiers = modifierRule(start, end);
    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});

    beforeRecover = start;
    recoverUntil(start, end, {TokenType::Identifier, TokenType::Semicolon}, false);
    if (beforeRecover != start) {
        auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end) {
        return std::nullopt;
    }
    if (start->isToken(TokenType::Semicolon)) {
        if (beforeRecover == start) {
            auto error = CompilerError(MissingPropertyName, start->getStart());
            addError(std::move(error));
        }
        start += 1;
        return std::nullopt;
    }

    auto name = identifierRule(start, end);
    if (!name) {
        auto error = CompilerError(MissingPropertyName, start->getStart());
        addError(std::move(error));
        return std::nullopt;
    }

    PropertyDeclaration prop(*name);
    if (!modifiers.empty()) {
        prop.startPos = modifiers.front().start;
    } else {
        prop.startPos = prop.name.start();
    }
    if (containsModifier(modifiers, TokenType::Mut)) {
        prop.isMutable = true;
    }
    if (containsModifier(modifiers, TokenType::Pub)) {
        prop.isPublic = true;
    }

    prop.endPos = name->start();

    beforeRecover = start;
    recoverUntil(start, end, [&](const TokenTreeNode &node) {
        return node.isSignatureStarter() || node.isToken(TokenType::Colon) || node.isToken(TokenType::Semicolon);
    }, false);
    if (beforeRecover != start) {
        auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end) {
        auto error = CompilerError(MissingPropertyType, start->getStart());
        addError(std::move(error));
        return std::move(prop);
    }
    if (start->isToken(TokenType::Colon)) {
        prop.endPos = start->getEnd();
        start += 1;
        beforeRecover = start;
        recoverUntil(start, end, [&](const TokenTreeNode &node) {
            return node.isSignatureStarter() || node.isToken(TokenType::Semicolon);
        }, false);
        if (beforeRecover != start) {
            auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
        if (start == end) {
            auto error = CompilerError(MissingPropertyType, start->getStart());
            addError(std::move(error));
            return std::nullopt;
        }
    }
    if (!start->isSignatureStarter()) {
        auto error = CompilerError(MissingPropertyType, start->getStart());
        addError(std::move(error));
        return std::move(prop);
    }

    auto signature = signatureRule(start, end);
    if (!signature) {
        auto error = CompilerError(MissingPropertyType, start->getStart());
        error.addLabel("this is not a valid type", start->getToken());
        addError(std::move(error));
        return std::move(prop);
    }

    prop.endPos = (*signature)->end();
    prop.type = std::move(signature);

    beforeRecover = start;
    recoverUntil(start, end, [&](const TokenTreeNode &node) {
        return node.isModifier() || node.isToken(TokenType::Identifier) || node.isToken(TokenType::Semicolon);
    }, false);

    if (beforeRecover != start) {
        auto error = CompilerError(UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, (start - 1)->getStart());
        addError(std::move(error));
    }
    prop.endPos = start->getEnd();
    start += 1;
    return std::move(prop);
}

void Parser::functionRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken(TokenType::Fn),
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

    if (start == end) {
        auto error = CompilerError(UnexpectedEndOfInput, decl.start());
        modules.back().functionDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected function name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(UnexpectedEndOfInput, decl.start());
        modules.back().functionDeclarations.emplace_back(std::move(decl));
        return;
    }

    //TODO:
    // ('<' genericParamList '>')?

    // '(' parameterList ')'

    // ('->' signature)?

    // genericConstraint*
}

void Parser::aliasRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken(TokenType::Alias),
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

    if (start == end) {
        auto error = CompilerError(UnexpectedEndOfInput, decl.start());
        addError(std::move(error));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected alias name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(UnexpectedEndOfInput, decl.start());
        addError(std::move(error));
        modules.back().aliasDeclarations.emplace_back(std::move(decl));
        return;
    }

    //TODO:

    // ('<' genericParamList '>')?

    // =

    // signature

    // genericConstraint*

    // ;
}

void Parser::moduleVariableRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    COMPILER_ASSERT(start->isToken(TokenType::Let),
                    "moduleVariableRule called with non-let starting token");

    auto decl = ModuleVariableDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = start->getStart();
    }
    decl.endPos = start->getEnd();
    start += 1;

    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);
    decl.isMut = containsModifier(modifiers, TokenType::Mut);

    if (start == end) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected variable name", *(start - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(MissingDeclarationName, decl.startPos);
        error.addLabel("expected variable name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(MissingSemicolon, decl.startPos);
        error.addLabel("missing semicolon", *(start - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!start->isToken(TokenType::Colon)) {
        auto error = CompilerError(UnexpectedToken, decl.startPos);
        error.addLabel("expected colon: `:`", *start);
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.endPos = start->getEnd();
    start += 1;

    if (start == end) {
        auto error = CompilerError(MissingVariableType, decl.startPos);
        error.addLabel("expected a type signature", *(start - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.type = signatureRule(start, end);

    if (!decl.type) {
        auto error = CompilerError(MissingVariableType, decl.startPos);
        error.addLabel("expected a type signature", *start);
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start == end) {
        auto error = CompilerError(UnexpectedToken, (start - 1)->getStart());
        error.setNote("unexpected end of method declaration, expected `;`");
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(MissingSemicolon, decl.start());
        error.addLabel("Missing semicolon", *start);
        addError(std::move(error));
    } else {
        decl.endPos = start->getEnd();
        start += 1;
    }

    modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
}

void Parser::declarationRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start->isModifier() || start->isDeclaratorKeyword(),
                    "declarationRule called with non-declaration starting token");

    auto modifiers = modifierRule(start, end);

    const auto &currentTokenTree = *start;
    if (currentTokenTree.isTokenResult()) {
        const auto &tokenResult = currentTokenTree.getTokenResult();
        if (tokenResult.isError()) {
            auto error = CompilerError(UnexpectedToken, start->getStart());
            error.addLabel("expected a top level declaration", *start);
            addError(std::move(error));
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
            addError(std::move(error));
            recoverTopLevel(start, end);
        }
    } else {
        auto error = CompilerError(UnexpectedToken, start->getStart());
        error.addLabel("expected a top level declaration", *start);
        addError(std::move(error));
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
                addError(std::move(error));
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

ConstraintDeclaration Parser::genericConstraintRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start != end && start->isToken(TokenType::Where),
                    "constraint rule called but start token is not a `where`");
    start += 1;
    auto decl = ConstraintDeclaration();

    decl.name = std::move(identifierRule(start, end));
    if (!decl.name) {
        auto error = CompilerError(UnexpectedToken, decl.startPos);
        error.addLabel("expected generic parameter name", *start);
        addError(std::move(error));
    }

    if (!start->isToken(TokenType::Colon)) {
        auto error = CompilerError(UnexpectedToken, decl.startPos);
        error.addLabel("expected `:`", *start);
        addError(std::move(error));
    } else {
        start += 1;
    }

    while (start != end) {
        if (auto c1 = interfaceConstraintRule(start, end)) {
            decl.constraints.emplace_back(std::make_unique<InterfaceConstraint>(std::move(*c1)));
        } /* else if (auto c2 = defaultConstraintRule(start, end)) {
            decl.constraints.push_back(std::move(c2));
        }*/
        else if (start->isConstraintBreakout()) {
            break;
        } else {
            auto error = CompilerError(InvalidGenericConstraint, decl.startPos);
            error.addLabel("expected generic constraint", *start);
            addError(std::move(error));
            recoverUntil(start, end, [](const TokenTreeNode &node) {
                return node.isConstraintBreakout() || node.isToken(TokenType::Comma);
            }, false);
        }

        if (start != end && start->isToken(TokenType::Comma)) {
            start += 1;
        } else {
            break;
        }
    }


    decl.endPos = start->getEnd();
    return decl;
}

std::optional<Identifier> Parser::identifierRule(treeIterator &start, const treeIterator &end) const {
    const auto current = start;
    if (!current->isToken(TokenType::Identifier)) {
        return std::nullopt;
    }

    start += 1;
    return Identifier::make(current->getToken(), source);
}

std::optional<std::unique_ptr<SignatureBase> > Parser::signatureRule(treeIterator &start, const treeIterator &end) {
    if (start == end) {
        return std::nullopt;
    }

    if (start->isToken(TokenType::Identifier) || start->isToken(TokenType::PathSeparator)) {
        auto type = typeSignatureRule(start, end);
        if (type) {
            return std::make_unique<TypeSignature>(std::move(*type));
        }
    } else if (start->isToken(TokenType::Fn)) {
        auto fn = functionSignatureRule(start, end);
        if (fn) {
            return std::make_unique<FunctionSignature>(std::move(*fn));
        }
    } else if (start->isTokenTree(TokenType::OpenParen)) {
        auto tuple = tupleSignatureRule(start, end);
        if (tuple) {
            return std::make_unique<TupleSignature>(std::move(*tuple));
        }
    }

    return std::nullopt;
}

std::optional<TypeSignature> Parser::typeSignatureRule(treeIterator &start, const treeIterator &end) {
    if (start == end || !start->isToken(TokenType::Identifier)) {
        return std::nullopt;
    }

    auto decl = TypeSignature();
    decl.startPos = start->getStart();

    decl.path = std::move(*pathRule(start, end, false));
    decl.endPos = decl.path.end();

    if (start == end) {
        return std::move(decl);
    }

    if (start->isTokenTree(TokenType::OpenAngle)) {
        decl.endPos = start->getEnd();
        decl.genericArguments = std::move(signatureListRule(*start, TokenType::OpenAngle));
        start += 1;
    }

    return std::move(decl);
}

std::optional<FunctionSignature> Parser::functionSignatureRule(
    treeIterator &start, const treeIterator &end) {
    if (start == end || !start->isToken(TokenType::Fn)) {
        return std::nullopt;
    }

    auto decl = FunctionSignature();
    decl.startPos = start->getStart();
    decl.endPos = start->getEnd();
    start += 1;

    if (start == end) {
        auto error = CompilerError(FnSignatureMissingParams, decl.start());
        error.addLabel("expected function parameters", (start - 1)->getEnd(), (start - 1)->getEnd());
        addError(std::move(error));
        return std::move(decl);
    }

    if (!start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(FnSignatureMissingParams, decl.start());
        error.addLabel("expected function parameters", *start);
        addError(std::move(error));
    } else {
        decl.parameterTypes = signatureListRule(*start, TokenType::OpenParen);
        start += 1;
    }

    decl.returnType = signatureRule(start, end);
    if (decl.returnType) {
        decl.endPos = (*decl.returnType)->end();
    }

    return std::move(decl);
}

std::optional<TupleSignature> Parser::tupleSignatureRule(treeIterator &start, const treeIterator &end) {
    if (start == end || !start->isTokenTree(TokenType::OpenParen)) {
        return std::nullopt;
    }

    auto decl = TupleSignature();
    decl.startPos = start->getStart();
    decl.endPos = start->getEnd();

    decl.types = signatureListRule(*start, TokenType::OpenParen);
    start += 1;

    return std::move(decl);
}

std::optional<ReturnType> Parser::returnTypeRule(treeIterator &start, const treeIterator &end) {
    if (start == end) {
        return std::nullopt;
    }

    ReturnType result;

    auto modifiers = modifierRule(start, end);
    validateModifiers(modifiers, {TokenType::Mut});
    result.isMut = containsModifier(modifiers, TokenType::Mut);

    if (!modifiers.empty()) {
        result.startPos = modifiers.front().start;
        result.endPos = modifiers.back().end;
    }

    auto type = signatureRule(start, end);
    if (!type) {
        return std::nullopt;
    }

    result.type = std::move(*type);
    if (modifiers.empty()) {
        result.endPos = result.type->end();
    }

    return std::move(result);
}

std::vector<Identifier> Parser::identifierListRule(const TokenTreeNode &node, TokenType opener) {
    std::vector<Identifier> result;

    if (!node.isTokenTree()) {
        auto error = CompilerError(UnexpectedToken, node.getStart());
        error.addLabel("unexpected token", node);
        error.setNote("expected a " + TokenTypeStringQuoted(opener));
        addError(std::move(error));
        return result;
    }

    const auto &list = node.getTokenTree();

    if (list.left.type != opener) {
        auto error = CompilerError(WrongOpener, list.left);
        error.addLabel("wrong opener for list, expected: " + TokenTypeStringQuoted(opener), list.left);
        addError(std::move(error));
    } else if (list.right.isError()) {
        auto error = CompilerError(WrongCloser, list.left);
        error.addLabel(
            "wrong closer for list, expected: " + TokenTypeStringQuoted(list.left.expectedClosing()),
            list.right.getError().got);
        addError(std::move(error));
    }

    treeIterator current = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();

    while (current != end) {
        auto identifier = identifierRule(current, end);
        if (!identifier) {
            auto error = CompilerError(UnexpectedToken, list.left);
            error.addLabel("expected " + TokenTypeStringQuoted(TokenType::Identifier) + " in list", *current);
            addError(std::move(error));

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
                addError(std::move(error));
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
    COMPILER_ASSERT(node.isTokenTree(opener), "parameterListRule called with the wrong node");

    const auto &list = node.getTokenTree();

    std::vector<Parameter> result;

    if (list.right.isError()) {
        auto error = CompilerError(WrongCloser, list.left);
        error.addLabel(
            "wrong closer for list, expected: " + TokenTypeStringQuoted(list.left.expectedClosing()),
            list.right.getError().got);
        addError(std::move(error));
    }

    treeIterator current = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();

    while (current != end) {
        auto startPos = current->getStart();
        auto modifiers = modifierRule(current, end);
        validateModifiers(modifiers, {TokenType::Mut, TokenType::Ref});

        auto identifier = identifierRule(current, end);
        if (!identifier) {
            auto error = CompilerError(ParameterNameMissing, node.getStart());
            error.addLabel("missing parameter name", *current);
            addError(std::move(error));
            recoverUntil(current, end, TokenType::Comma, true);
            continue;
        }

        if (current == end) {
            auto error = CompilerError(ParameterTypeMissing, node.getStart());
            error.addLabel("expected `:`", *(current - 1));
            addError(std::move(error));
            return std::move(result);
        }

        if (!current->isToken(TokenType::Colon)) {
            auto error = CompilerError(UnexpectedToken, node.getStart());
            error.addLabel("expected `:`", *current);
            addError(std::move(error));
        } else {
            current += 1;
        }

        auto signature = signatureRule(current, end);
        if (!signature) {
            auto error = CompilerError(ParameterTypeMissing, node.getStart());
            error.addLabel("missing parameter type", *current);
            addError(std::move(error));
            recoverUntil(current, end, TokenType::Comma, true);
            continue;
        }
        auto endPos = (*signature)->end();
        auto &param = result.emplace_back(containsModifier(modifiers, TokenType::Mut),
                                          containsModifier(modifiers, TokenType::Ref),
                                          std::move(*identifier),
                                          std::move(*signature));
        param.startPos = startPos;
        param.endPos = endPos;

        if (result.back().isMut && result.back().isRef) {
            auto error = CompilerError(RefAlreadyImpliesMut, modifiers.front());
            error.setNote("redundant `mut` specification, `ref` implies `mut`");
            addError(std::move(error));
        }

        if (current != end) {
            if (current->isToken(TokenType::Comma)) {
                current += 1;
            } else {
                auto error = CompilerError(MissingComma, list.left);
                error.addLabel("expected a comma", *current);
                addError(std::move(error));
            }
        }
    }

    return std::move(result);
}

std::vector<std::unique_ptr<SignatureBase> > Parser::signatureListRule(const TokenTreeNode &node, TokenType opener) {
    if (!node.isTokenTree()) {
        auto error = CompilerError(UnexpectedToken, node.getStart());
        error.addLabel("unexpected token", node);
        error.setNote("expected a " + TokenTypeStringQuoted(opener));
        addError(std::move(error));
    }

    const auto &list = node.getTokenTree();

    std::vector<std::unique_ptr<SignatureBase> > result;

    if (list.left.type != opener) {
        auto error = CompilerError(WrongOpener, list.left);
        error.addLabel("wrong opener for list, expected: " + TokenTypeStringQuoted(opener), list.left);
        addError(std::move(error));
    } else if (list.right.isError()) {
        auto error = CompilerError(WrongCloser, list.left);
        error.addLabel(
            "wrong closer for list, expected: " + TokenTypeStringQuoted(list.left.expectedClosing()),
            list.right.getError().got);
        addError(std::move(error));
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
            addError(std::move(error));
            recoverUntil(current, end, [](const TokenTreeNode &node) {
                return node.isSignatureStarter() || node.isToken(TokenType::Comma);
            }, false);
        }

        if (current != end) {
            if (current->isToken(TokenType::Comma)) {
                current += 1;
            } else {
                auto error = CompilerError(MissingComma, list.left);
                error.addLabel("expected a comma", *current);
                addError(std::move(error));
            }
        }
    }

    return std::move(result);
}

void Parser::recoverTopLevel(treeIterator &start, const treeIterator &end) {
    recoverUntil(start, end, [](const TokenTreeNode &node) {
        return node.isToken(TokenType::Semicolon)
               || node.isTopLevelStarter();
    }, false);

    if (start->isToken(TokenType::Semicolon)) {
        start += 1;
    }
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, TokenType type, const bool consume) {
    recoverUntil(start, end, [type](const TokenTreeNode &node) {
        return node.isToken(type);
    }, consume);
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, std::vector<TokenType> oneOf,
                          const bool consume) {
    recoverUntil(start, end, [&oneOf](const TokenTreeNode &node) {
        return node.isToken() && std::ranges::find(oneOf, node.getToken().type) != oneOf.end();
    }, consume);
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end,
                          const std::function<bool(const TokenTreeNode &)> &predicate, const bool consume) {
    while (start != end) {
        if (predicate(*start)) {
            if (consume) {
                start += 1;
            }
            break;
        }

        start += 1;
    }
}

std::optional<InterfaceConstraint> Parser::interfaceConstraintRule(treeIterator &start, const treeIterator &end) {
    InterfaceConstraint constraint;
    auto type = typeSignatureRule(start, end);
    if (!type) {
        return std::nullopt;
    }
    constraint.typeSignature = std::move(*type);
    return constraint;
}

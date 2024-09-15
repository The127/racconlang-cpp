//
// Created by zoe on 07.09.24.
//

#include "Parser.h"

#include <bits/ranges_algo.h>

#include <utility>

#include "sourceMap/Source.h"
#include "errors/InternalError.h"
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
#include "errors/ErrorContext.h"

Parser::Parser(const std::shared_ptr<Source> &source)
    : source(source) {
    modules.emplace_back();
    uses = std::make_shared<FileUses>();
}

Parser::Parser(Parser &&) noexcept = default;
Parser &Parser::operator=(Parser &&) noexcept = default;

Parser::~Parser() = default;

void Parser::parse() {
    parseFile();

    for (auto &module: modules) {
        module.uses = uses;
    }

    source->modules = std::move(modules);
}

void Parser::addError(CompilerError error) {
    source->addError(std::move(error));
}

void Parser::parseFile() {
    COMPILER_ASSERT(source->tokenTree, "Token tree was not set: " + source->fileName);

    const auto &tokenTree = *source->tokenTree;
    COMPILER_ASSERT(tokenTree.left.type == TokenType::Bof, "Token tree did not start with BOF: " + source->fileName);
    COMPILER_ASSERT(tokenTree.right.isToken(TokenType::Eof), "Token tree did not end with an EOF: " + source->fileName);

    treeIterator start = tokenTree.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = tokenTree.tokens.end();

    while (start != end) {
        recoverTopLevel(start, end);
        if (start == end) {
            break;
        }
        if (start->isToken(TokenType::Use)) {
            useRule(start, end);
        } else if (start->isToken(TokenType::Mod)) {
            modRule(start, end);
        } else if (start->isModifier() || start->isDeclaratorKeyword()) {
            declarationRule(start, end);
        } else {
            COMPILER_ASSERT(false, std::format("unhandled top level declaration, got {}", start->debugString()));
        }
    }
}

void Parser::useRule(treeIterator &start, const treeIterator &end) {
    if (modules.size() > 1) {
        auto error = CompilerError(ErrorCode::UseAfterMod, start->getStart());
        error.setNote("uses need to be declared before any modules");
        addError(std::move(error));
    }

    const auto &useToken = consumeToken(start, end, TokenType::Use);


    recoverUntil(start, end, [](const auto &ttn) {
        return ttn.isTopLevelStarter()
               || ttn.isPathStarter();
    });

    if (start == end || start->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::UseIsMissingPath, (start - 1)->getEnd()));
        return;
    }

    auto &use = uses->uses.emplace_back();
    use.startPos = useToken.start;

    use.path = pathRule(start, end, true);
    use.endPos = use.path.end();

    recoverUntil(start, end, [](const auto &n) {
        return n.isTopLevelStarter() || n.isToken(TokenType::Semicolon) || n.isTokenTree(TokenType::OpenCurly);
    });

    if (start == end || start->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd()));
        return;
    }
    if (const auto semicolon = tryConsumeToken(start, end, TokenType::Semicolon)) {
        use.endPos = semicolon->end;
        return;
    }

    if (const auto tree = tryConsumeTokenTree(start, end, TokenType::OpenCurly)) {
        if (!use.path.isTrailing()) {
            addError(CompilerError(ErrorCode::MissingPathSeparator, (*tree)->left.start));
        }
        use.names = std::move(identifierListRule(**tree));
        use.endPos = (*tree)->right.getEnd();
    }


    recoverUntil(start, end, [](const auto &n) {
        return n.isTopLevelStarter() || n.isToken(TokenType::Semicolon);
    });

    if (start == end || start->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd()));
        return;
    }
    const auto semicolon = consumeToken(start, end, TokenType::Semicolon);
    use.endPos = semicolon.end;
}

void Parser::modRule(treeIterator &start, const treeIterator &end) {
    const auto modToken = consumeToken(start, end, TokenType::Mod);

    recoverUntil(start, end, [](const auto &n) {
        return n.isTopLevelStarter() || n.isPathStarter() || n.isToken(TokenType::Semicolon);
    });
    if (start == end || start->isTopLevelStarter() || start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingModulePath, start->getStart());
        addError(std::move(error));
        tryConsumeToken(start, end, TokenType::Semicolon);
        return;
    }

    auto &mod = modules.emplace_back();
    mod.startPos = modToken.start;

    mod.path = pathRule(start, end, false);

    mod.endPos = mod.path.end();
    recoverUntil(start, end, [](const auto &n) {
        return n.isTopLevelStarter() || n.isToken(TokenType::Semicolon);
    });

    if (start == end || !start->isToken(TokenType::Semicolon)) {
        addError(CompilerError(ErrorCode::MissingSemicolon, start->getStart()));
    } else {
        mod.endPos = start->getEnd();
        consumeToken(start, end, TokenType::Semicolon);
    }
}

std::vector<Token> Parser::modifierRule(treeIterator &start, const treeIterator &end,
                                        const RecoverPredicate &recoverPredicate) {
    std::vector<Token> result;

    while (start != end) {
        recoverUntil(start, end, [&recoverPredicate](const auto &n) {
            return n.isModifier() || recoverPredicate(n);
        });
        if (start == end || !start->isModifier()) {
            break;
        }

        COMPILER_ASSERT(start->isToken(), "current node is not a token");
        const auto &token = start->getToken();

        auto alreadyContained = std::ranges::find_if(result, [&token](auto &t) {
            return t.type == token.type;
        });

        if (alreadyContained != result.end()) {
            auto error = CompilerError(ErrorCode::DuplicateModifier, start->getStart());
            error.addLabel("duplicate use of this modifier", *start);
            error.addLabel("modifier is already present here", *alreadyContained);
            error.setNote("a modifier is only allowed once per declaration");
            addError(std::move(error));
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
            auto error = CompilerError(ErrorCode::InvalidModifier, modifiers[0]);
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
    auto &decl = modules.back().enumDeclarations.emplace_back();
    const auto enumToken = consumeToken(start, end, TokenType::Enum);

    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
    } else {
        decl.startPos = enumToken.start;
    }
    decl.endPos = enumToken.end;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    recoverUntil(start, end, [](const auto &n) {
        return n.isTopLevelStarter()
               || n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenAngle)
               || n.isToken(TokenType::Where)
               || n.isTokenTree(TokenType::OpenCurly);
    });

    if (start == end || start->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (start - 1)->getEnd()));
        return;
    }

    if (!start->isToken(TokenType::Identifier)) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (start - 1)->getEnd()));
    } else {
        decl.name = identifierRule(start, end);
        decl.endPos = decl.name->end();

        recoverUntil(start, end, [](const auto &n) {
            return n.isTopLevelStarter() || n.isTokenTree(TokenType::OpenAngle) || n.isToken(TokenType::Where) || n.
                   isTokenTree(TokenType::OpenCurly);
        });

        if (start == end || start->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (start - 1)->getEnd()));
            return;
        }
    }


    if (const auto generics = tryConsumeTokenTree(start, end, TokenType::OpenAngle)) {
        decl.endPos = (*generics)->right.getEnd();
        decl.genericParams = identifierListRule(**generics);

        recoverUntil(start, end, [](const auto &n) {
            return n.isTopLevelStarter() || n.isToken(TokenType::Where) || n.isTokenTree(TokenType::OpenCurly);
        });

        if (start == end || start->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (start - 1)->getEnd()));
            return;
        }
    }


    // wrong
    if (start->isToken(TokenType::Where)) {
        decl.genericConstraints = genericConstraintListRule(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isTokenTree(TokenType::OpenCurly);
        });
        if (!decl.genericConstraints.empty()) {
            decl.endPos = decl.genericConstraints.back().end();
        }

        recoverUntil(start, end, [](const auto &n) {
            return n.isTopLevelStarter() || n.isTokenTree(TokenType::OpenCurly);
        });

        if (start == end || start->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (start - 1)->getEnd()));
            return;
        }
    }


    COMPILER_ASSERT(start->isTokenTree(TokenType::OpenCurly), "unexpected token");

    decl.endPos = start->getEnd();
    const auto &body = start->getTokenTree();

    if (body.right.isError()) {
        addError(CompilerError(ErrorCode::UnclosedEnumBody, body.right.getStart()));
    }

    treeIterator bodyStart = body.tokens.begin(); // NOLINT(*-use-auto)
    const auto bodyEnd = body.tokens.end();

    while (bodyStart != bodyEnd) {
        auto member = enumMemberRule(bodyStart, bodyEnd);
        if (member) {
            decl.memberDeclarations.emplace_back(std::move(*member));
            recoverUntil(bodyStart, bodyEnd, {TokenType::Identifier, TokenType::Comma});
            if (start != end && start->isToken(TokenType::Identifier)) {
                addError(CompilerError(ErrorCode::MissingComma, start->getStart()));
            } else {
                tryConsumeToken(start, end, TokenType::Comma);
            }
        }
    }

    start += 1;
}

std::optional<EnumMemberDeclaration> Parser::enumMemberRule(treeIterator &start, const treeIterator &end) {
    recoverUntil(start, end, {TokenType::Identifier, TokenType::Comma});

    if (start == end || tryConsumeToken(start, end, TokenType::Comma)) {
        return std::nullopt;
    }

    auto decl = EnumMemberDeclaration(identifierRule(start, end));

    decl.startPos = decl.name.start();
    decl.endPos = decl.name.end();

    if (start == end) {
        return std::move(decl);
    }

    if (const auto &tree = tryConsumeTokenTree(start, end, TokenType::OpenParen)) {
        decl.endPos = (*tree)->right.getEnd();
        decl.values = signatureListRule(**tree);
    }
    return std::move(decl);
}


void Parser::interfaceRule(treeIterator &start, const treeIterator &end, std::vector<Token> modifiers) {
    auto &decl = modules.back().interfaceDeclarations.emplace_back();
    auto interfaceToken = consumeToken(start, end, TokenType::Interface);

    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
    } else {
        decl.startPos = interfaceToken.start;
    }
    decl.endPos = interfaceToken.end;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    recoverUntil(start, end, [](const auto &n) {
        return n.isTopLevelStarter()
               || n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenAngle)
               || n.isTokenTree(TokenType::Colon)
               || n.isTokenTree(TokenType::Where)
               || n.isTokenTree(TokenType::OpenCurly);
    });

    if (start == end || start->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (start - 1)->getEnd()));
        return;
    }

    if (!start->isToken(TokenType::Identifier)) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (start - 1)->getEnd()));
    } else {
        decl.name = identifierRule(start, end);
        decl.endPos = decl.name->end();

        recoverUntil(start, end, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(TokenType::OpenAngle)
                   || n.isTokenTree(TokenType::Colon)
                   || n.isTokenTree(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (start == end || start->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingInterfaceBody, (start - 1)->getEnd()));
            return;
        }
    }


    if (const auto &generics = tryConsumeTokenTree(start, end, TokenType::OpenAngle)) {
        decl.endPos = (*generics)->right.getEnd();
        decl.genericParams = identifierListRule(**generics);

        recoverUntil(start, end, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(TokenType::Colon)
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (start == end || start->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingInterfaceBody, (start - 1)->getEnd()));
            return;
        }
    }

    if (const auto &colon = tryConsumeToken(start, end, TokenType::Colon)) {
        decl.endPos = colon->end;

        while (start != end) {
            recoverUntil(start, end, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isPathStarter()
                       || n.isTokenTree(TokenType::Where)
                       || n.isTokenTree(TokenType::OpenCurly);
            });

            if (start == end || !start->isPathStarter()) {
                break;
            }

            const auto &signature = decl.requiredInterfaces.emplace_back(typeSignatureRule(start, end));
            decl.endPos = signature.end();

            recoverUntil(start, end, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(TokenType::Comma)
                       || n.isPathStarter()
                       || n.isToken(TokenType::Where)
                       || n.isTokenTree(TokenType::OpenCurly);
            });
            if (auto tok = tryConsumeToken(start, end, TokenType::Comma)) {
                decl.endPos = tok->end;
            } else if (start->isPathStarter()) {
                addError(CompilerError(ErrorCode::MissingComma, (start - 1)->getEnd()));
            } else {
                break;
            }
        }

        recoverUntil(start, end, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (start == end || start->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingInterfaceBody, (start - 1)->getEnd()));
            return;
        }
    }


    if (start->isToken(TokenType::Where)) {
        decl.genericConstraints = genericConstraintListRule(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isTokenTree(TokenType::OpenCurly);
        });
        if (!decl.genericConstraints.empty()) {
            decl.endPos = decl.genericConstraints.back().end();
        }

        recoverUntil(start, end, [](const auto &n) {
            return n.isTopLevelStarter() || n.isTokenTree(TokenType::OpenCurly);
        });

        if (start == end || start->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (start - 1)->getEnd()));
            return;
        }
    }


    // here
    if (!start->isToken(TokenType::Where)
        && !start->isTokenTree(TokenType::OpenCurly)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.startPos);
        error.addLabel(
            "unexpected token, expected: constraints (`where`), interface body (`{`)",
            *start);
        addError(std::move(error));

        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::Where)
                   || node.isTokenTree(TokenType::OpenCurly);
        });

        if (start == end) {
            modules.back().interfaceDeclarations.emplace_back(std::move(decl));
            return;
        }
    }


    if (start->isTopLevelStarter()) {
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }


    decl.genericConstraints = genericConstraintListRule(start, end, [](const TokenTreeNode &node) {
        return node.isTokenTree(TokenType::OpenCurly)
               || node.isTopLevelStarter();
    });

    if (!decl.genericConstraints.empty()) {
        decl.endPos = decl.genericConstraints.back().end();
    }

    if (start == end || !start->isTokenTree(TokenType::OpenCurly)) {
        auto error = CompilerError(ErrorCode::MissingInterfaceBody, decl.startPos);
        error.addLabel("missing interface body", *(start - 1));
        addError(std::move(error));
        modules.back().interfaceDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.endPos = start->getEnd();

    const auto &tokenTree = start->getTokenTree();
    if (tokenTree.right.isError()) {
        auto error = CompilerError(ErrorCode::WrongCloser, tokenTree.left);
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
            memberModifiers = modifierRule(bodyStart, bodyEnd, [](const auto &n) {
                return n.isToken(TokenType::Fn);
            });
            validateModifiers(memberModifiers, {TokenType::Pub, TokenType::Mut});
        }
        if (bodyStart == bodyEnd) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, originalStart->getStart());
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
            auto error = CompilerError(ErrorCode::UnexpectedToken, originalStart->getStart());
            error.addLabel("unexpected token here", *bodyStart);
            recoverUntil(bodyStart, bodyEnd, [](const TokenTreeNode &node) {
                return node.isModifier()
                       || node.isToken(TokenType::Get)
                       || node.isToken(TokenType::Set);
            });
        }
    }

    start += 1;


    modules.back().interfaceDeclarations.emplace_back(std::move(decl));
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
        auto error = CompilerError(ErrorCode::MissingMethodName, (start - 1)->getEnd());
        error.setNote("unexpected end of method declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingMethodName, decl.startPos);
        error.addLabel("expected method name", *start);
        addError(std::move(error));
        return std::nullopt;
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (start - 1)->getEnd());
        error.setNote("unexpected end of method declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    if (!start->isTokenTree(TokenType::OpenAngle) && !start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.start());
        error.addLabel("expected generic parameters or method parameters", *start);
        addError(std::move(error));
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTokenTree(TokenType::OpenParen)
                   || node.isTokenTree(TokenType::OpenAngle)
                   || node.isToken(TokenType::Semicolon);
        });
        if (start->isToken(TokenType::Semicolon)) {
            start += 1;
            return std::nullopt;
        }
        if (start == end) {
            return std::nullopt;
        }
    }

    if (const auto tree = tryConsumeTokenTree(start, end, TokenType::OpenAngle)) {
        decl.endPos = (*tree)->right.getEnd();
        decl.genericParams = std::move(identifierListRule(**tree));
    }

    if (start == end) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (start - 1)->getEnd());
        error.setNote("unexpected end of method declaration, expected method parameters");
        addError(std::move(error));
        return std::nullopt;
    }

    if (!start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.start());
        error.addLabel("expected method parameters", *start);
        addError(std::move(error));
        return std::nullopt;
    }
    const auto &tree = consumeTokenTree(start, end, TokenType::OpenParen);
    decl.parameters = parameterListRule(tree);

    if (start == end) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (start - 1)->getEnd());
        error.setNote("unexpected end of method declaration, expected return type, generic constraints or `;`");
        addError(std::move(error));
        return std::make_optional(std::move(decl));
    }

    if (start->isToken(TokenType::DashArrow)) {
        start += 1;
        if (start == end) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, (start - 1)->getEnd());
            error.setNote("unexpected end of method declaration, expected return type, generic constraints or `;`");
            addError(std::move(error));
            return std::nullopt;
        }

        decl.returnType = returnTypeRule(start, end);
        if (!decl.returnType) {
            auto error = CompilerError(ErrorCode::MissingMethodReturnType, decl.start());
            error.addLabel("expected method return type", *start);
            addError(std::move(error));
            return std::nullopt;
        }
    }


    decl.genericConstraints = genericConstraintListRule(start, end, [](const TokenTreeNode &node) {
        return node.isToken(TokenType::Semicolon)
               || node.isTopLevelStarter();
    });

    if (!decl.genericConstraints.empty()) {
        decl.endPos = decl.genericConstraints.back().end();
    }

    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd());
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
        auto error = CompilerError(ErrorCode::MissingGetterName, (start - 1)->getEnd());
        error.setNote("unexpected end of getter declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingGetterName, decl.startPos);
        error.addLabel("expected getter name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end || !start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::MissingGetterParam, (start - 1)->getEnd());
        error.setNote("unexpected end of setter declaration, expected `(`");
        addError(std::move(error));
        return std::move(decl);
    }

    const auto &tree = consumeTokenTree(start, end, TokenType::OpenParen);
    decl.endPos = tree.right.getEnd();
    auto params = parameterListRule(tree);

    if (!params.empty()) {
        auto error = CompilerError(ErrorCode::TooManyGetterParams, params[0].start());
        error.setNote("getters cannot have parameters");
        addError(std::move(error));
    }

    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingGetterReturnType, (start - 1)->getEnd());
        error.setNote("unexpected end of getter declaration, expected return type`->`");
        addError(std::move(error));
        return std::move(decl);
    }

    if (!start->isToken(TokenType::DashArrow)) {
        auto error = CompilerError(ErrorCode::MissingGetterReturnType, decl.startPos);
        error.addLabel("unexpected end of getter declaration, expected return type: `->`", *start);
        addError(std::move(error));
        return std::move(decl);
    }

    decl.endPos = start->getEnd();
    start += 1;

    decl.returnType = returnTypeRule(start, end);
    if (!decl.returnType) {
        auto error = CompilerError(ErrorCode::MissingGetterReturnType, decl.startPos);
        error.addLabel("unexpected end of getter declaration, expected return type", *start);
        addError(std::move(error));
        return std::move(decl);
    }

    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    if (start->isToken(TokenType::Semicolon)) {
        decl.endPos = start->getEnd();
        start += 1;
    } else {
        auto error = CompilerError(ErrorCode::MissingSemicolon, start->getStart());
        error.setNote("expected a semicolon");
        addError(std::move(error));
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
        auto error = CompilerError(ErrorCode::MissingSetterName, (start - 1)->getEnd());
        error.setNote("unexpected end of setter declaration");
        addError(std::move(error));
        return std::nullopt;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingSetterName, decl.startPos);
        error.addLabel("expected setter name", *start);
        addError(std::move(error));
        return std::nullopt;
    }
    decl.endPos = decl.name->end();

    if (start == end || !start->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::MissingSetterParam, (start - 1)->getEnd());
        error.setNote("unexpected end of setter declaration, expected `(`");
        addError(std::move(error));
        return std::move(decl);
    }

    const auto &tree = consumeTokenTree(start, end, TokenType::OpenParen);

    decl.endPos = tree.right.getEnd();
    auto params = parameterListRule(tree);

    if (params.size() > 1) {
        auto error = CompilerError(ErrorCode::TooManySetterParams, params[1].start());
        error.setNote("setters must have exactly 1 parameter");
        addError(std::move(error));
    }

    if (params.empty()) {
        auto error = CompilerError(ErrorCode::MissingSetterParam, (start - 1)->getEnd());
        error.setNote("setters must have exactly 1 parameter");
        addError(std::move(error));
    } else {
        decl.parameter = std::move(params[0]);
    }

    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    if (start->isToken(TokenType::Semicolon)) {
        decl.endPos = start->getEnd();
        start += 1;
    } else {
        auto error = CompilerError(ErrorCode::MissingSemicolon, start->getStart());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    return std::move(decl);
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
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected struct name", *start);
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected struct name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, decl.startPos);
        error.addLabel("expected semicolon", *start);
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (const auto tree = tryConsumeTokenTree(start, end, TokenType::OpenAngle)) {
        decl.endPos = (*tree)->right.getEnd();
        decl.genericParams = std::move(identifierListRule(**tree));
    }

    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingStructBody, decl.startPos);
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
    });
    if (beforeRecover != start) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end || start->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *(start - 1));
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }


    decl.genericConstraints = genericConstraintListRule(start, end, [](const TokenTreeNode &node) {
        return node.isTokenTree(TokenType::OpenCurly)
               || node.isTokenTree(TokenType::OpenParen)
               || node.isTopLevelStarter();
    });

    if (!decl.genericConstraints.empty()) {
        decl.endPos = decl.genericConstraints.back().end();
    }

    if (start == end || start->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *(start - 1));
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (const auto tree = tryConsumeTokenTree(start, end, TokenType::OpenParen)) {
        auto parameters = parameterListRule(**tree);
        for (auto &parameter: parameters) {
            if (parameter.isMut || parameter.isRef) {
                auto error = CompilerError(ErrorCode::InvalidModifier, parameter.startPos);
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
        decl.endPos = (*tree)->right.getEnd();

        beforeRecover = start;
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::Semicolon)
                   || node.isToken(TokenType::Where);
        });
        if (beforeRecover != start) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
        if (start == end || start->isTopLevelStarter()) {
            auto error = CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd());
            addError(std::move(error));
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }

        decl.genericConstraints = genericConstraintListRule(start, end, [](const TokenTreeNode &node) {
            return node.isToken(TokenType::Semicolon)
                   || node.isTopLevelStarter();
        });

        if (start == end || !start->isToken(TokenType::Semicolon)) {
            auto error = CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd());
            addError(std::move(error));
        }
        start += 1;
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
        });
        if (start != end && start->isToken(TokenType::DestructuresInto)) {
            decl.endPos = start->getEnd();
            start += 1;
            if (start == end) {
                auto error = CompilerError(ErrorCode::UnexpectedEndOfInput, (start - 1)->getEnd());
                addError(std::move(error));
                modules.back().structDeclarations.emplace_back(std::move(decl));
                return;
            }
            decl.destructureProperties = identifierListRule(consumeTokenTree(start, end, TokenType::OpenParen));


            beforeRecover = start;
            recoverUntil(start, end, [](const TokenTreeNode &node) {
                return node.isTopLevelStarter()
                       || node.isToken(TokenType::Semicolon);
            });

            if (beforeRecover != start) {
                auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
                addError(std::move(error));
            }
            if (start == end || !start->isToken(TokenType::Semicolon)) {
                auto error = CompilerError(ErrorCode::MissingSemicolon, beforeRecover->getStart());
                addError(std::move(error));
            }
            start += 1;
        }
    } else {
        COMPILER_ASSERT(false, "unreachable");
    }

    modules.back().structDeclarations.emplace_back(std::move(decl));
}

std::vector<ConstraintDeclaration> Parser::genericConstraintListRule(treeIterator &start, const treeIterator &end,
                                                                     const RecoverPredicate &recoverPredicate) {
    std::vector<ConstraintDeclaration> result;
    while (start != end && start->isToken(TokenType::Where)) {
        auto constraint = genericConstraintRule(start, end);
        if (constraint) {
            result.emplace_back(std::move(*constraint));
        }
        auto beforeRecover = start;
        recoverUntil(start, end, [&recoverPredicate](const TokenTreeNode &node) {
            return node.isToken(TokenType::Where) || recoverPredicate(node);
        });
        if (beforeRecover != start) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
    }
    return result;
}


std::optional<PropertyDeclaration> Parser::propertyDeclarationRule(treeIterator &start, const treeIterator &end) {
    auto beforeRecover = start;
    recoverUntil(start, end, [](const TokenTreeNode &node) {
        return node.isModifier() || node.isToken(TokenType::Identifier) || node.isToken(TokenType::Semicolon);
    });
    if (beforeRecover != start) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end) {
        //BEAN
        return std::nullopt;
    }
    if (start->isToken(TokenType::Semicolon)) {
        if (beforeRecover == start) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, start->getStart());
            addError(std::move(error));
        }
        start += 1;
        return std::nullopt;
    }

    auto modifiers = modifierRule(start, end, [](const auto &) { return true; });
    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});

    beforeRecover = start;
    recoverUntil(start, end, {TokenType::Identifier, TokenType::Semicolon});
    if (beforeRecover != start) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end) {
        return std::nullopt;
    }
    if (start->isToken(TokenType::Semicolon)) {
        if (beforeRecover == start) {
            auto error = CompilerError(ErrorCode::MissingPropertyName, start->getStart());
            addError(std::move(error));
        }
        start += 1;
        return std::nullopt;
    }
    if (!start->isToken(TokenType::Identifier)) {
        auto error = CompilerError(ErrorCode::MissingPropertyName, start->getStart());
        addError(std::move(error));
        return std::nullopt;
    }

    PropertyDeclaration prop(identifierRule(start, end));
    if (!modifiers.empty()) {
        prop.startPos = modifiers.front().start;
    } else {
        prop.startPos = prop.name.start();
    }
    prop.isMutable = containsModifier(modifiers, TokenType::Mut);
    prop.isPublic = containsModifier(modifiers, TokenType::Pub);

    prop.endPos = prop.name.start();

    beforeRecover = start;
    recoverUntil(start, end, [&](const TokenTreeNode &node) {
        return node.isSignatureStarter() || node.isToken(TokenType::Colon) || node.isToken(TokenType::Semicolon);
    });
    if (beforeRecover != start) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingPropertyType, start->getStart());
        addError(std::move(error));
        return std::move(prop);
    }
    if (start->isToken(TokenType::Colon)) {
        prop.endPos = start->getEnd();
        start += 1;
        beforeRecover = start;
        recoverUntil(start, end, [&](const TokenTreeNode &node) {
            return node.isSignatureStarter() || node.isToken(TokenType::Semicolon);
        });
        if (beforeRecover != start) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
        if (start == end) {
            auto error = CompilerError(ErrorCode::MissingPropertyType, start->getStart());
            addError(std::move(error));
            return std::nullopt;
        }
    }
    if (!start->isSignatureStarter()) {
        auto error = CompilerError(ErrorCode::MissingPropertyType, start->getStart());
        addError(std::move(error));
        return std::move(prop);
    }

    auto signature = signatureRule(start, end);

    prop.endPos = signature->end();
    prop.type = std::move(signature);

    beforeRecover = start;
    recoverUntil(start, end, [&](const TokenTreeNode &node) {
        return node.isModifier() || node.isToken(TokenType::Identifier) || node.isToken(TokenType::Semicolon);
    });

    if (beforeRecover != start) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (start == end || !start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd());
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
        auto error = CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start());
        modules.back().functionDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected function name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start());
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

    auto beforeRecover = start;
    recoverUntil(start, end, [](const TokenTreeNode &node) {
        return node.isTopLevelStarter() || node.isToken(TokenType::Identifier);
    });

    if (start == end || start->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (start - 1)->getEnd()));
        return;
    }

    if (beforeRecover != start) {
        addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected alias name", *start);
        addError(std::move(error));
        return;
    }
    decl.endPos = decl.name->end();

    beforeRecover = start;
    recoverUntil(start, end, [](const TokenTreeNode &node) {
        return node.isTopLevelStarter() || node.isSignatureStarter() || node.isToken(TokenType::Equals) || node.
               isTokenTree(TokenType::OpenAngle);
    });

    if (start == end || start->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingAliasType, decl.start());
        addError(std::move(error));
        modules.back().aliasDeclarations.emplace_back(std::move(decl));
        return;
    }
    if (start != beforeRecover) {
        addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
    }
    if (const auto tree = tryConsumeTokenTree(start, end, TokenType::OpenAngle)) {
        decl.genericParams = identifierListRule(**tree);

        beforeRecover = start;
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isSignatureStarter() || node.isToken(TokenType::Equals);
        });

        if (start == end || start->isTopLevelStarter()) {
            auto error = CompilerError(ErrorCode::MissingAliasType, decl.start());
            addError(std::move(error));
            modules.back().aliasDeclarations.emplace_back(std::move(decl));
            return;
        }
        if (start != beforeRecover) {
            addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
    }
    if (start->isToken(TokenType::Equals)) {
        start += 1;
        beforeRecover = start;
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isSignatureStarter();
        });

        if (start == end || start->isTopLevelStarter()) {
            auto error = CompilerError(ErrorCode::MissingAliasType, decl.start());
            addError(std::move(error));
            modules.back().aliasDeclarations.emplace_back(std::move(decl));
            return;
        }
        if (start != beforeRecover) {
            addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
    } else {
        auto error = CompilerError(ErrorCode::MissingEquals, decl.start());
        addError(std::move(error));
    }


    auto signature = signatureRule(start, end);
    if (!signature) {
        modules.back().aliasDeclarations.emplace_back(std::move(decl));
        return;
    }
    decl.signature = std::move(signature);
    decl.endPos = (*decl.signature)->end();


    decl.genericConstraints = genericConstraintListRule(start, end, [](const TokenTreeNode &node) {
        return node.isToken(TokenType::Semicolon)
               || node.isTopLevelStarter();
    });
    if (!decl.genericConstraints.empty()) {
        decl.endPos = decl.genericConstraints.back().end();
    }

    if (start == end || start->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (start - 1)->getEnd());
    } else {
        COMPILER_ASSERT(start->isToken(TokenType::Semicolon), "unexpected token type");
        start += 1;
    }

    modules.back().aliasDeclarations.emplace_back(std::move(decl));
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
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected variable name", *(start - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(start, end);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected variable name", *start);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, decl.startPos);
        error.addLabel("missing semicolon", *(start - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!start->isToken(TokenType::Colon)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.startPos);
        error.addLabel("expected colon: `:`", *start);
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.endPos = start->getEnd();
    start += 1;

    if (start == end) {
        auto error = CompilerError(ErrorCode::MissingVariableType, decl.startPos);
        error.addLabel("expected a type signature", *(start - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.type = signatureRule(start, end);

    if (!decl.type) {
        auto error = CompilerError(ErrorCode::MissingVariableType, decl.startPos);
        error.addLabel("expected a type signature", *start);
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (start == end) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (start - 1)->getEnd());
        error.setNote("unexpected end of method declaration, expected `;`");
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!start->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, decl.start());
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

    auto modifiers = modifierRule(start, end, [](const auto &) { return true; });

    const auto &currentTokenTree = *start;
    if (currentTokenTree.isTokenResult()) {
        const auto &tokenResult = currentTokenTree.getTokenResult();
        if (tokenResult.isError()) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, start->getStart());
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
            auto error = CompilerError(ErrorCode::UnexpectedToken, start->getStart());
            error.addLabel("expected a top level declaration", *start);
            addError(std::move(error));
            recoverTopLevel(start, end);
        }
    } else {
        auto error = CompilerError(ErrorCode::UnexpectedToken, start->getStart());
        error.addLabel("expected a top level declaration", *start);
        addError(std::move(error));
        recoverTopLevel(start, end);
    }
}

Path Parser::pathRule(treeIterator &start, const treeIterator &end, const bool allowTrailing) {
    Path path{};

    if ((path.rooted = tryConsumeToken(start, end, TokenType::PathSeparator))) {
        recoverUntil(start, end, TokenType::Identifier);
        if (start == end) {
            addError(CompilerError(ErrorCode::EmptyPath, path.rooted->start));
            return path;
        }
    }

    path.parts.push_back(identifierRule(start, end));

    while (start != end && start->isToken(TokenType::PathSeparator)) {
        const auto fallback = start;
        auto trailer = consumeToken(start, end, TokenType::PathSeparator);
        if (start == end || !start->isToken(TokenType::Identifier)) {
            if (allowTrailing) {
                path.trailer = trailer;
            } else {
                start = fallback;
            }
            break;
        }

        path.parts.push_back(identifierRule(start, end));
    }
    return path;
}

std::optional<ConstraintDeclaration> Parser::genericConstraintRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start != end && start->isToken(TokenType::Where),
                    "constraint rule called but start token is not a `where`");
    start += 1;
    auto decl = ConstraintDeclaration();

    // TODO: error recovery?

    decl.name = std::move(identifierRule(start, end));
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.startPos);
        error.addLabel("expected generic parameter name", *start);
        addError(std::move(error));
        return std::nullopt;
    }
    decl.startPos = decl.name->start();
    decl.endPos = decl.name->end();

    // TODO: error recovery and end checking
    if (!start->isToken(TokenType::Colon)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.startPos);
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
            auto error = CompilerError(ErrorCode::InvalidGenericConstraint, decl.startPos);
            error.addLabel("expected generic constraint", *start);
            addError(std::move(error));
            recoverUntil(start, end, [](const TokenTreeNode &node) {
                return node.isConstraintBreakout() || node.isToken(TokenType::Comma);
            });
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

std::optional<InterfaceConstraint> Parser::interfaceConstraintRule(treeIterator &start, const treeIterator &end) {
    InterfaceConstraint constraint;
    constraint.typeSignature = typeSignatureRule(start, end);
    return constraint;
}

Identifier Parser::identifierRule(treeIterator &start, const treeIterator &end) const {
    return Identifier::make(consumeToken(start, end, TokenType::Identifier), source);
}

std::unique_ptr<SignatureBase> Parser::signatureRule(treeIterator &start, const treeIterator &end) {
    COMPILER_ASSERT(start != end && start->isSignatureStarter(), "signatureRule called with non signature starter");

    if (start->isPathStarter()) {
        return std::make_unique<TypeSignature>(typeSignatureRule(start, end));
    }
    if (start->isToken(TokenType::Fn)) {
        return std::make_unique<FunctionSignature>(functionSignatureRule(start, end));
    }
    if (start->isTokenTree(TokenType::OpenParen)) {
        return std::make_unique<TupleSignature>(tupleSignatureRule(start, end));
    }

    COMPILER_ASSERT(false, "signature rule called with wrong input");
}

TypeSignature Parser::typeSignatureRule(treeIterator &start, const treeIterator &end) {
    auto decl = TypeSignature();

    decl.path = std::move(pathRule(start, end, false));
    decl.startPos = decl.path.start();
    decl.endPos = decl.path.end();

    if (start == end) {
        return decl;
    }

    if (const auto tree = tryConsumeTokenTree(start, end, TokenType::OpenAngle)) {
        decl.genericArguments = std::move(signatureListRule(**tree));
        decl.endPos = (*tree)->right.getEnd();
    }

    return decl;
}

FunctionSignature Parser::functionSignatureRule(treeIterator &start, const treeIterator &end) {
    const auto fnToken = consumeToken(start, end, TokenType::Fn);
    auto decl = FunctionSignature();
    decl.startPos = fnToken.start;
    decl.endPos = fnToken.end;

    if (start == end) {
        addError(CompilerError(ErrorCode::FnSignatureMissingParams, decl.start()));
        return decl;
    }

    if (const auto parens = tryConsumeTokenTree(start, end, TokenType::OpenParen)) {
        decl.parameterTypes = signatureListRule(**parens);
        decl.endPos = (*parens)->right.getEnd();

        if (start == end) {
            addError(CompilerError(ErrorCode::FnSignatureMissingParams, decl.start()));
            return decl;
        }
    } else {
        addError(CompilerError(ErrorCode::MissingMethodReturnType, start->getStart()));
    }

    if (const auto dashArrow = tryConsumeToken(start, end, TokenType::DashArrow)) {
        decl.endPos = dashArrow->end;

        if (start->isSignatureStarter()) {
            decl.returnType = returnTypeRule(start, end);
            decl.endPos = (*decl.returnType).end();
        } else {
            addError(CompilerError(ErrorCode::MissingMethodReturnType, dashArrow->start));
        }
    }

    return decl;
}

TupleSignature Parser::tupleSignatureRule(treeIterator &start, const treeIterator &end) {
    auto decl = TupleSignature();

    auto const &list = consumeTokenTree(start, end, TokenType::OpenParen);
    decl.startPos = list.left.start;
    decl.endPos = list.right.getEnd();

    decl.types = signatureListRule(list);

    return decl;
}

ReturnType Parser::returnTypeRule(treeIterator &start, const treeIterator &end) {
    ReturnType result;

    auto modifiers = modifierRule(start, end, [](const auto &n) {
        return true;
    });
    validateModifiers(modifiers, {TokenType::Mut});
    result.isMut = containsModifier(modifiers, TokenType::Mut);

    if (!modifiers.empty()) {
        result.startPos = modifiers.front().start;
        result.endPos = modifiers.back().end;
    }

    if (start == end || !start->isSignatureStarter()) {
        addError(CompilerError(ErrorCode::MissingMethodReturnType, start->getStart()));
        return result;
    }

    result.type = signatureRule(start, end);
    if (modifiers.empty()) {
        result.startPos = result.type->start();
    }
    result.endPos = result.type->end();

    return std::move(result);
}

std::vector<Identifier> Parser::identifierListRule(const TokenTree &list) {
    std::vector<Identifier> result;

    treeIterator start = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();

    while (start != end) {
        recoverUntil(start, end, {TokenType::Identifier, TokenType::Comma});
        if (start == end) {
            break;
        }

        if (start->isToken(TokenType::Identifier)) {
            result.emplace_back(identifierRule(start, end));
            recoverUntil(start, end, {TokenType::Identifier, TokenType::Comma});
            if (start == end) {
                break;
            }
        } else {
            addError(CompilerError(ErrorCode::UnexpectedToken, start->getStart()));
            start += 1;
            continue;
        }

        if (!tryConsumeToken(start, end, TokenType::Comma)) {
            addError(CompilerError(ErrorCode::MissingComma, start->getStart()));
        }
    }

    return result;
}

std::vector<Parameter> Parser::parameterListRule(const TokenTree &list) {
    std::vector<Parameter> result;

    treeIterator current = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();
    while (current != end) {
        auto startPos = current->getStart();
        auto modifiers = modifierRule(current, end, [](const auto &) { return true; });
        validateModifiers(modifiers, {TokenType::Mut, TokenType::Ref});

        auto beforeRecover = current;
        recoverUntil(current, end, [](const TokenTreeNode &node) {
            return node.isToken(TokenType::Identifier) || node.isToken(TokenType::Comma);
        });
        if (beforeRecover != current) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            error.addLabel("unexpected token", *beforeRecover);
            addError(std::move(error));
        }
        if (current == end || !current->isToken(TokenType::Identifier)) {
            addError(CompilerError(ErrorCode::ParameterNameMissing, (current - 1)->getEnd()));
            continue;
        }

        auto identifier = identifierRule(current, end);

        auto &param = result.emplace_back(identifier);
        param.startPos = startPos;
        param.endPos = identifier.end();
        param.isMut = containsModifier(modifiers, TokenType::Mut);
        param.isRef = containsModifier(modifiers, TokenType::Ref);
        if (param.isMut && param.isRef) {
            auto error = CompilerError(ErrorCode::RefAlreadyImpliesMut, modifiers.front());
            error.setNote("redundant `mut` specification, `ref` implies `mut`");
            addError(std::move(error));
        }

        beforeRecover = current;
        recoverUntil(current, end, [](const TokenTreeNode &n) {
            return n.isSignatureStarter() || n.isToken(TokenType::Colon) || n.isToken(TokenType::Comma);
        });
        if (beforeRecover != current) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            error.addLabel("unexpected token", *beforeRecover);
            addError(std::move(error));
        }
        if (current == end) {
            addError(CompilerError(ErrorCode::ParameterTypeMissing, (current - 1)->getEnd()));
            continue;
        }
        if (current->isToken(TokenType::Comma)) {
            addError(CompilerError(ErrorCode::ParameterTypeMissing, current->getStart()));
            param.endPos = current->getEnd();
            current += 1;
            continue;
        }

        if (!current->isToken(TokenType::Colon)) {
            auto error = CompilerError(ErrorCode::MissingColon, current->getStart());
            error.addLabel("expected `:` to separate the parameter name and type", *current);
            addError(std::move(error));
        } else {
            param.endPos = current->getEnd();
            current += 1;
            beforeRecover = current;
            recoverUntil(current, end, [](const TokenTreeNode &node) {
                return node.isSignatureStarter() || node.isToken(TokenType::Comma);
            });
            if (beforeRecover != current) {
                auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
                error.addLabel("unexpected token", *beforeRecover);
                addError(std::move(error));
            }
            if (current == end || current->isToken(TokenType::Comma)) {
                addError(CompilerError(ErrorCode::ParameterTypeMissing, (current - 1)->getEnd()));
                continue;
            }
        }
        COMPILER_ASSERT(current != end && current->isSignatureStarter(),
                        "current must be a signature starter at this point");


        param.type = signatureRule(current, end);
        if (param.type) {
            param.endPos = (*param.type)->end();
        }
        beforeRecover = current;
        recoverUntil(current, end, TokenType::Comma); // TODO: move unexpected token error into recoverUntil?
        if (current != beforeRecover) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            error.addLabel("unexpected token", *beforeRecover);
            addError(std::move(error));
        }

        DEBUG_ASSERT(current == end || current->isToken(TokenType::Comma), "expeced end or comma");

        if (current != end) {
            current += 1;
        }
    }

    return std::move(result);
}

std::vector<std::unique_ptr<SignatureBase> > Parser::signatureListRule(const TokenTree &list) {
    std::vector<std::unique_ptr<SignatureBase> > result;

    treeIterator start = list.tokens.begin(); // NOLINT(*-use-auto)
    const auto end = list.tokens.end();

    while (start != end) {
        if (start->isSignatureStarter()) {
            result.emplace_back(signatureRule(start, end));
        } else {
            auto error = CompilerError(ErrorCode::InvalidSignature, list.left);
            error.addLabel("expected type, tuple or function signature", *start);
            addError(std::move(error));
            recoverUntil(start, end, [](const TokenTreeNode &node) {
                return node.isSignatureStarter() || node.isToken(TokenType::Comma);
            });
        }

        if (start != end) {
            if (start->isToken(TokenType::Comma)) {
                start += 1;
            } else {
                auto error = CompilerError(ErrorCode::MissingComma, list.left);
                error.addLabel("expected a comma", *start);
                addError(std::move(error));
            }
        }
    }

    return std::move(result);
}

void Parser::recoverTopLevel(treeIterator &start, const treeIterator &end) {
    while (start != end) {
        recoverUntil(start, end, [](const TokenTreeNode &node) {
            return node.isToken(TokenType::Semicolon)
                   || node.isTopLevelStarter()
                   || node.isModifier();
        });
        if (!tryConsumeToken(start, end, TokenType::Semicolon)) {
            break;
        }
    }
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, TokenType type) {
    recoverUntil(start, end, [type](const TokenTreeNode &node) {
        return node.isToken(type);
    });
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, std::vector<TokenType> oneOf) {
    recoverUntil(start, end, [&oneOf](const TokenTreeNode &node) {
        return node.isToken() && std::ranges::find(oneOf, node.getToken().type) != oneOf.end();
    });
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, const RecoverPredicate &predicate) {
    auto beforeRecover = start;
    while (start != end && !predicate(*start)) {
        start += 1;
    }

    if (beforeRecover != start) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
}

void Parser::recoverUntil(treeIterator &start, const treeIterator &end, const RecoverPredicate &predicate,
                          ErrorContext &errCtx) {
    const auto beforeRecover = start;
    while (start != end && !predicate(*start)) {
        start += 1;
    }

    if (beforeRecover != start) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        errCtx.addError(std::move(error));
    }
}

Token Parser::consumeToken(treeIterator &start, const treeIterator &end, const TokenType type) {
    COMPILER_ASSERT(start != end, std::format("trying to consume {} on empty iterator", TokenTypeName(type)));
    COMPILER_ASSERT(start->isToken(type),
                    std::format("trying to consume {} but got {}", TokenTypeName(type), start->debugString()));

    auto const token = start->getToken();
    start += 1;
    return token;
}

std::optional<Token> Parser::tryConsumeToken(treeIterator &start, const treeIterator &end, const TokenType type) {
    if (start == end || !start->isToken(type)) {
        return std::nullopt;
    }

    auto token = start->getToken();
    start += 1;
    return token;
}

auto Parser::consumeTokenTree(treeIterator &start, const treeIterator &end, TokenType type) -> const TokenTree & {
    COMPILER_ASSERT(start != end,
                    std::format("trying to consume TokenTree[{}] on empty iterator", TokenTypeName(type)));
    COMPILER_ASSERT(start->isTokenTree(type),
                    std::format("trying to consume TokenTree[{}] but got {}", TokenTypeName(type), start->debugString()
                    ));

    auto const &tree = start->getTokenTree();

    if (tree.right.isError()) {
        auto error = CompilerError(ErrorCode::WrongCloser, tree.right.getOrErrorToken());
        error.setNote("unclosed list, expected: " + TokenTypeStringQuoted(tree.left.expectedClosing()));
        addError(std::move(error));
    }

    start += 1;
    return tree;
}

std::optional<const TokenTree *> Parser::tryConsumeTokenTree(treeIterator &start, const treeIterator &end,
                                                             const TokenType type) {
    if (start == end || !start->isTokenTree(type)) {
        return std::nullopt;
    }

    auto const &tree = start->getTokenTree();

    if (tree.right.isError()) {
        auto error = CompilerError(ErrorCode::WrongCloser, tree.right.getOrErrorToken());
        error.setNote("unclosed list: expected " + TokenTypeStringQuoted(tree.left.expectedClosing()));
        addError(std::move(error));
    }

    start += 1;
    return &tree;
}

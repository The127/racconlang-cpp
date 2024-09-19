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
#include "ast/ConstructorDeclaration.h"
#include "ast/DestructureDeclaration.h"
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
#include "ast/ImplBlock.h"
#include "ast/ImplGetter.h"
#include "ast/ImplMethod.h"
#include "ast/ImplSetter.h"
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

    auto it = TokenTreeIterator(tokenTree.tokens);

    while (it) {
        recoverTopLevel(it);
        if (it.isEnd()) {
            break;
        }
        if (it->isToken(TokenType::Use)) {
            useRule(it);
        } else if (it->isToken(TokenType::Mod)) {
            modRule(it);
        } else if (it->isModifier() || it->isDeclaratorKeyword()) {
            declarationRule(it);
        } else if (it->isToken(TokenType::Impl)) {
            implRule(it);
        } else {
            COMPILER_ASSERT(false, std::format("unhandled top level declaration, got {}", it->debugString()));
        }
    }
}

void Parser::useRule(TokenTreeIterator &it) {
    if (modules.size() > 1) {
        auto error = CompilerError(ErrorCode::UseAfterMod, it->getStart());
        error.setNote("uses need to be declared before any modules");
        addError(std::move(error));
    }

    const auto &useToken = consumeToken(it, TokenType::Use);


    recoverUntil(it, [](const auto &ttn) {
        return ttn.isTopLevelStarter()
               || ttn.isPathStarter();
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::UseIsMissingPath, (it - 1)->getEnd()));
        return;
    }

    auto &use = uses->uses.emplace_back();
    use.startPos = useToken.start;

    use.path = pathRule(it, true);
    use.endPos = use.path.end();

    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter() || n.isToken(TokenType::Semicolon) || n.isTokenTree(TokenType::OpenCurly);
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
        return;
    }
    if (const auto semicolon = tryConsumeToken(it, TokenType::Semicolon)) {
        use.endPos = semicolon->end;
        return;
    }

    if (const auto tree = tryConsumeTokenTree(it, TokenType::OpenCurly)) {
        if (!use.path.isTrailing()) {
            addError(CompilerError(ErrorCode::MissingPathSeparator, (*tree)->left.start));
        }
        use.names = std::move(identifierListRule(**tree));
        use.endPos = (*tree)->right.getEnd();
    }


    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter() || n.isToken(TokenType::Semicolon);
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
        return;
    }
    const auto semicolon = consumeToken(it, TokenType::Semicolon);
    use.endPos = semicolon.end;
}

void Parser::modRule(TokenTreeIterator &it) {
    const auto modToken = consumeToken(it, TokenType::Mod);

    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter() || n.isPathStarter() || n.isToken(TokenType::Semicolon);
    });
    if (it.isEnd() || it->isTopLevelStarter() || it->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingModulePath, it->getStart());
        addError(std::move(error));
        tryConsumeToken(it, TokenType::Semicolon);
        return;
    }

    auto &mod = modules.emplace_back();
    mod.startPos = modToken.start;

    mod.path = pathRule(it, false);

    mod.endPos = mod.path.end();
    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter() || n.isToken(TokenType::Semicolon);
    });

    if (it.isEnd() || !it->isToken(TokenType::Semicolon)) {
        addError(CompilerError(ErrorCode::MissingSemicolon, it->getStart()));
    } else {
        mod.endPos = it->getEnd();
        consumeToken(it, TokenType::Semicolon);
    }
}

std::vector<Token> Parser::modifierRule(TokenTreeIterator &it,
                                        const RecoverPredicate &recoverPredicate) {
    std::vector<Token> result;

    while (it) {
        recoverUntil(it, [&recoverPredicate](const auto &n) {
            return n.isModifier() || recoverPredicate(n);
        });
        if (it.isEnd() || !it->isModifier()) {
            break;
        }

        COMPILER_ASSERT(it->isToken(), "current node is not a token");
        const auto &token = it->getToken();

        auto alreadyContained = std::ranges::find_if(result, [&token](auto &t) {
            return t.type == token.type;
        });

        if (alreadyContained != result.end()) {
            auto error = CompilerError(ErrorCode::DuplicateModifier, it->getStart());
            error.addLabel("duplicate use of this modifier", *it);
            error.addLabel("modifier is already present here", *alreadyContained);
            error.setNote("a modifier is only allowed once per declaration");
            addError(std::move(error));
        } else {
            result.emplace_back(it->getToken());
        }

        it += 1;
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

void Parser::enumRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    auto &decl = modules.back().enumDeclarations.emplace_back();
    const auto enumToken = consumeToken(it, TokenType::Enum);

    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
    } else {
        decl.startPos = enumToken.start;
    }
    decl.endPos = enumToken.end;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter()
               || n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenAngle)
               || n.isToken(TokenType::Where)
               || n.isTokenTree(TokenType::OpenCurly);
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
        return;
    }

    if (!it->isToken(TokenType::Identifier)) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
    } else {
        decl.name = identifierRule(it);
        decl.endPos = decl.name->end();

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter() || n.isTokenTree(TokenType::OpenAngle) || n.isToken(TokenType::Where) || n.
                   isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
            return;
        }
    }


    if (const auto generics = tryConsumeTokenTree(it, TokenType::OpenAngle)) {
        decl.endPos = (*generics)->right.getEnd();
        decl.genericParams = identifierListRule(**generics);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter() || n.isToken(TokenType::Where) || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
            return;
        }
    }


    if (it->isToken(TokenType::Where)) {
        decl.genericConstraints = genericConstraintListRule(it, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isTokenTree(TokenType::OpenCurly);
        });
        if (!decl.genericConstraints.empty()) {
            decl.endPos = decl.genericConstraints.back().end();
        }

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter() || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
            return;
        }
    }


    COMPILER_ASSERT(it->isTokenTree(TokenType::OpenCurly), "unexpected token");

    decl.endPos = it->getEnd();
    const auto &body = it->getTokenTree();

    if (body.right.isError()) {
        addError(CompilerError(ErrorCode::UnclosedEnumBody, body.right.getStart()));
    }

    auto bodyIt = TokenTreeIterator(body.tokens);

    while (bodyIt) {
        auto member = enumMemberRule(bodyIt);
        if (member) {
            decl.memberDeclarations.emplace_back(std::move(*member));
            recoverUntil(bodyIt, {TokenType::Identifier, TokenType::Comma});
            if (it && it->isToken(TokenType::Identifier)) {
                addError(CompilerError(ErrorCode::MissingComma, it->getStart()));
            } else {
                tryConsumeToken(it, TokenType::Comma);
            }
        }
    }

    it += 1;
}

std::optional<EnumMemberDeclaration> Parser::enumMemberRule(TokenTreeIterator &it) {
    recoverUntil(it, {TokenType::Identifier, TokenType::Comma});

    if (it.isEnd() || tryConsumeToken(it, TokenType::Comma)) {
        return std::nullopt;
    }

    auto decl = EnumMemberDeclaration(identifierRule(it));

    decl.startPos = decl.name.start();
    decl.endPos = decl.name.end();

    if (it.isEnd()) {
        return std::move(decl);
    }

    if (const auto &tree = tryConsumeTokenTree(it, TokenType::OpenParen)) {
        decl.endPos = (*tree)->right.getEnd();
        decl.values = signatureListRule(**tree);
    }
    return std::move(decl);
}


void Parser::interfaceRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    auto &decl = modules.back().interfaceDeclarations.emplace_back();
    auto interfaceToken = consumeToken(it, TokenType::Interface);

    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
    } else {
        decl.startPos = interfaceToken.start;
    }
    decl.endPos = interfaceToken.end;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter()
               || n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenAngle)
               || n.isToken(TokenType::Colon)
               || n.isToken(TokenType::Where)
               || n.isTokenTree(TokenType::OpenCurly);
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
        return;
    }

    if (!it->isToken(TokenType::Identifier)) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
    } else {
        decl.name = identifierRule(it);
        decl.endPos = decl.name->end();

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(TokenType::OpenAngle)
                   || n.isToken(TokenType::Colon)
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingInterfaceBody, (it - 1)->getEnd()));
            return;
        }
    }


    if (const auto &generics = tryConsumeTokenTree(it, TokenType::OpenAngle)) {
        decl.endPos = (*generics)->right.getEnd();
        decl.genericParams = identifierListRule(**generics);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(TokenType::Colon)
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingInterfaceBody, (it - 1)->getEnd()));
            return;
        }
    }

    if (const auto &colon = tryConsumeToken(it, TokenType::Colon)) {
        decl.endPos = colon->end;

        while (it) {
            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isPathStarter()
                       || n.isTokenTree(TokenType::Where)
                       || n.isTokenTree(TokenType::OpenCurly);
            });

            if (it.isEnd() || !it->isPathStarter()) {
                break;
            }

            const auto &signature = decl.requiredInterfaces.emplace_back(typeSignatureRule(it));
            decl.endPos = signature.end();

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(TokenType::Comma)
                       || n.isPathStarter()
                       || n.isToken(TokenType::Where)
                       || n.isTokenTree(TokenType::OpenCurly);
            });
            if (auto tok = tryConsumeToken(it, TokenType::Comma)) {
                decl.endPos = tok->end;
            } else if (it->isPathStarter()) {
                addError(CompilerError(ErrorCode::MissingComma, (it - 1)->getEnd()));
            } else {
                break;
            }
        }

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingInterfaceBody, (it - 1)->getEnd()));
            return;
        }
    }


    while (it->isToken(TokenType::Where)) {
        decl.genericConstraints = genericConstraintListRule(it, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isTokenTree(TokenType::OpenCurly);
        });
        if (!decl.genericConstraints.empty()) {
            decl.endPos = decl.genericConstraints.back().end();
        }

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(TokenType::OpenCurly)
                   || n.isToken(TokenType::Where);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
            return;
        }
    }


    const auto &bodyTree = consumeTokenTree(it, TokenType::OpenCurly);
    auto bodyIt = TokenTreeIterator(bodyTree.tokens);

    while (bodyIt) {
        auto originalIt = bodyIt;

        recoverUntil(bodyIt, [](const auto &n) {
            return n.isModifier()
                   || n.isToken(TokenType::Fn)
                   || n.isToken(TokenType::Get)
                   || n.isToken(TokenType::Set);
        });

        if (bodyIt.isEnd()) {
            break;
        }

        std::vector<Token> memberModifiers;
        if (bodyIt->isModifier()) {
            memberModifiers = modifierRule(bodyIt, [](const auto &n) {
                return n.isToken(TokenType::Fn)
                       || n.isToken(TokenType::Get)
                       || n.isToken(TokenType::Set);
            });

            recoverUntil(bodyIt, [](const auto &n) {
                return n.isToken(TokenType::Fn)
                       || n.isToken(TokenType::Get)
                       || n.isToken(TokenType::Set);
            });

            if (bodyIt.isEnd()) {
                addError(CompilerError(ErrorCode::MissingInterfaceMember, originalIt->getStart()));
                break;
            }
        }

        if (bodyIt->isToken(TokenType::Fn)) {
            decl.methods.emplace_back(interfaceMethodRule(bodyIt, std::move(memberModifiers)));
        } else if (bodyIt->isToken(TokenType::Get)) {
            decl.getters.emplace_back(interfaceGetterRule(bodyIt, std::move(memberModifiers)));
        } else if (bodyIt->isToken(TokenType::Set)) {
            decl.setters.emplace_back(interfaceSetterRule(bodyIt, std::move(memberModifiers)));
        } else {
            addError(CompilerError(ErrorCode::UnexpectedToken, originalIt->getStart()));
        }
    }
}


InterfaceMethodDeclaration Parser::interfaceMethodRule(TokenTreeIterator &it,
                                                       std::vector<Token> modifiers) {
    auto fnToken = consumeToken(it, TokenType::Fn);
    auto decl = InterfaceMethodDeclaration();
    decl.startPos = fnToken.start;

    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});
    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
        decl.isMut = containsModifier(modifiers, TokenType::Mut);
    }

    if (it.isEnd()) {
        addError(CompilerError(ErrorCode::MissingMethodName, (it - 1)->getEnd()));
        return std::move(decl);
    }

    decl.name = identifierRule(it);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingMethodName, decl.startPos);
        error.addLabel("expected method name", *it);
        addError(std::move(error));
        return std::move(decl);
    }
    decl.endPos = decl.name->end();

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (it - 1)->getEnd());
        error.setNote("unexpected end of method declaration");
        addError(std::move(error));
        return std::move(decl);
    }

    if (!it->isTokenTree(TokenType::OpenAngle) && !it->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.start());
        error.addLabel("expected generic parameters or method parameters", *it);
        addError(std::move(error));
        recoverUntil(it, [](const TokenTreeNode &node) {
            return node.isTokenTree(TokenType::OpenParen)
                   || node.isTokenTree(TokenType::OpenAngle)
                   || node.isToken(TokenType::Semicolon);
        });
        if (it->isToken(TokenType::Semicolon)) {
            it += 1;
            return std::move(decl);
        }
        if (it.isEnd()) {
            return std::move(decl);
        }
    }

    if (const auto tree = tryConsumeTokenTree(it, TokenType::OpenAngle)) {
        decl.endPos = (*tree)->right.getEnd();
        decl.genericParams = std::move(identifierListRule(**tree));
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (it - 1)->getEnd());
        error.setNote("unexpected end of method declaration, expected method parameters");
        addError(std::move(error));
        return std::move(decl);
    }

    if (!it->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.start());
        error.addLabel("expected method parameters", *it);
        addError(std::move(error));
        return std::move(decl);
    }
    const auto &tree = consumeTokenTree(it, TokenType::OpenParen);
    decl.parameters = parameterListRule(tree);

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (it - 1)->getEnd());
        error.setNote("unexpected end of method declaration, expected return type, generic constraints or `;`");
        addError(std::move(error));
        return std::move(decl);
    }

    if (it->isToken(TokenType::DashArrow)) {
        it += 1;
        if (it.isEnd()) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, (it - 1)->getEnd());
            error.setNote("unexpected end of method declaration, expected return type, generic constraints or `;`");
            addError(std::move(error));
            return std::move(decl);
        }

        decl.returnType = returnTypeRule(it);
        if (!decl.returnType) {
            auto error = CompilerError(ErrorCode::MissingMethodReturnType, decl.start());
            error.addLabel("expected method return type", *it);
            addError(std::move(error));
            return std::move(decl);
        }
    }


    decl.genericConstraints = genericConstraintListRule(it, [](const TokenTreeNode &node) {
        return node.isToken(TokenType::Semicolon)
               || node.isTopLevelStarter();
    });

    if (!decl.genericConstraints.empty()) {
        decl.endPos = decl.genericConstraints.back().end();
    }

    if (it.isEnd() || !it->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd());
        addError(std::move(error));
        return std::move(decl);
    }

    decl.endPos = it->getEnd();
    it += 1;

    return std::move(decl);
}

InterfaceGetter Parser::interfaceGetterRule(TokenTreeIterator &it,
                                            std::vector<Token> modifiers) {
    auto getToken = consumeToken(it, TokenType::Get);
    auto decl = InterfaceGetter();
    decl.startPos = getToken.start;

    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});
    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
        decl.isMut = containsModifier(modifiers, TokenType::Mut);
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingGetterName, (it - 1)->getEnd());
        error.setNote("unexpected end of getter declaration");
        addError(std::move(error));
        return std::move(decl);
    }

    decl.name = identifierRule(it);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingGetterName, decl.startPos);
        error.addLabel("expected getter name", *it);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (it.isEnd() || !it->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::MissingGetterParam, (it - 1)->getEnd());
        error.setNote("unexpected end of setter declaration, expected `(`");
        addError(std::move(error));
        return std::move(decl);
    }

    const auto &tree = consumeTokenTree(it, TokenType::OpenParen);
    decl.endPos = tree.right.getEnd();
    auto params = parameterListRule(tree);

    if (!params.empty()) {
        auto error = CompilerError(ErrorCode::TooManyGetterParams, params[0].start());
        error.setNote("getters cannot have parameters");
        addError(std::move(error));
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingGetterReturnType, (it - 1)->getEnd());
        error.setNote("unexpected end of getter declaration, expected return type`->`");
        addError(std::move(error));
        return std::move(decl);
    }

    if (!it->isToken(TokenType::DashArrow)) {
        auto error = CompilerError(ErrorCode::MissingGetterReturnType, decl.startPos);
        error.addLabel("unexpected end of getter declaration, expected return type: `->`", *it);
        addError(std::move(error));
        return std::move(decl);
    }

    decl.endPos = it->getEnd();
    it += 1;

    decl.returnType = returnTypeRule(it);
    if (!decl.returnType) {
        auto error = CompilerError(ErrorCode::MissingGetterReturnType, decl.startPos);
        error.addLabel("unexpected end of getter declaration, expected return type", *it);
        addError(std::move(error));
        return std::move(decl);
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    if (it->isToken(TokenType::Semicolon)) {
        decl.endPos = it->getEnd();
        it += 1;
    } else {
        auto error = CompilerError(ErrorCode::MissingSemicolon, it->getStart());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    return std::move(decl);
}

InterfaceSetter Parser::interfaceSetterRule(TokenTreeIterator &it,
                                            std::vector<Token> modifiers) {
    auto setToken = consumeToken(it, TokenType::Set);
    auto decl = InterfaceSetter();
    decl.startPos = setToken.start;

    validateModifiers(modifiers, {TokenType::Pub});
    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingSetterName, (it - 1)->getEnd());
        error.setNote("unexpected end of setter declaration");
        addError(std::move(error));
        return std::move(decl);
    }

    decl.name = identifierRule(it);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingSetterName, decl.startPos);
        error.addLabel("expected setter name", *it);
        addError(std::move(error));
        return std::move(decl);
    }
    decl.endPos = decl.name->end();

    if (it.isEnd() || !it->isTokenTree(TokenType::OpenParen)) {
        auto error = CompilerError(ErrorCode::MissingSetterParam, (it - 1)->getEnd());
        error.setNote("unexpected end of setter declaration, expected `(`");
        addError(std::move(error));
        return std::move(decl);
    }

    const auto &tree = consumeTokenTree(it, TokenType::OpenParen);

    decl.endPos = tree.right.getEnd();
    auto params = parameterListRule(tree);

    if (params.size() > 1) {
        auto error = CompilerError(ErrorCode::TooManySetterParams, params[1].start());
        error.setNote("setters must have exactly 1 parameter");
        addError(std::move(error));
    }

    if (params.empty()) {
        auto error = CompilerError(ErrorCode::MissingSetterParam, (it - 1)->getEnd());
        error.setNote("setters must have exactly 1 parameter");
        addError(std::move(error));
    } else {
        decl.parameter = std::move(params[0]);
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    if (it->isToken(TokenType::Semicolon)) {
        decl.endPos = it->getEnd();
        it += 1;
    } else {
        auto error = CompilerError(ErrorCode::MissingSemicolon, it->getStart());
        error.setNote("expected a semicolon");
        addError(std::move(error));
    }

    return std::move(decl);
}


void Parser::structRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    COMPILER_ASSERT(it->isToken(TokenType::Struct), "structRule called with non-struct starting token");

    auto decl = StructDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = it->getStart();
    }
    decl.endPos = it->getEnd();
    it += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected struct name", *it);
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(it);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected struct name", *it);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, decl.startPos);
        error.addLabel("expected semicolon", *it);
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (const auto tree = tryConsumeTokenTree(it, TokenType::OpenAngle)) {
        decl.endPos = (*tree)->right.getEnd();
        decl.genericParams = std::move(identifierListRule(**tree));
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *it);
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    auto beforeRecover = it;
    recoverUntil(it, [](const TokenTreeNode &node) {
        return node.isTokenTree(TokenType::OpenCurly)
               || node.isTokenTree(TokenType::OpenParen)
               || node.isToken(TokenType::Where)
               || node.isTopLevelStarter();
    });
    if (beforeRecover != it) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (it.isEnd() || it->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *(it - 1));
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }


    decl.genericConstraints = genericConstraintListRule(it, [](const TokenTreeNode &node) {
        return node.isTokenTree(TokenType::OpenCurly)
               || node.isTokenTree(TokenType::OpenParen)
               || node.isTopLevelStarter();
    });

    if (!decl.genericConstraints.empty()) {
        decl.endPos = decl.genericConstraints.back().end();
    }

    if (it.isEnd() || it->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingStructBody, decl.startPos);
        error.addLabel("expected struct body", *(it - 1));
        addError(std::move(error));
        modules.back().structDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (const auto tree = tryConsumeTokenTree(it, TokenType::OpenParen)) {
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

        beforeRecover = it;
        recoverUntil(it, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::Semicolon)
                   || node.isToken(TokenType::Where);
        });
        if (beforeRecover != it) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
        if (it.isEnd() || it->isTopLevelStarter()) {
            auto error = CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd());
            addError(std::move(error));
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }

        decl.genericConstraints = genericConstraintListRule(it, [](const TokenTreeNode &node) {
            return node.isToken(TokenType::Semicolon)
                   || node.isTopLevelStarter();
        });

        if (it.isEnd() || !it->isToken(TokenType::Semicolon)) {
            auto error = CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd());
            addError(std::move(error));
        }
        it += 1;
    } else if (it->isTokenTree(TokenType::OpenCurly)) {
        const auto &body = it->getTokenTree();
        auto bodyIt = TokenTreeIterator(body.tokens);

        while (bodyIt) {
            auto prop = propertyDeclarationRule(bodyIt);
            if (prop) {
                decl.propertyDeclarations.emplace_back(std::move(*prop));
            }
        }

        it += 1;
        beforeRecover = it;
        recoverUntil(it, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter()
                   || node.isToken(TokenType::DestructuresInto);
        });
        if (it && it->isToken(TokenType::DestructuresInto)) {
            decl.endPos = it->getEnd();
            it += 1;
            if (it.isEnd()) {
                auto error = CompilerError(ErrorCode::UnexpectedEndOfInput, (it - 1)->getEnd());
                addError(std::move(error));
                modules.back().structDeclarations.emplace_back(std::move(decl));
                return;
            }
            decl.destructureProperties = identifierListRule(consumeTokenTree(it, TokenType::OpenParen));


            beforeRecover = it;
            recoverUntil(it, [](const TokenTreeNode &node) {
                return node.isTopLevelStarter()
                       || node.isToken(TokenType::Semicolon);
            });

            if (beforeRecover != it) {
                auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
                addError(std::move(error));
            }
            if (it.isEnd() || !it->isToken(TokenType::Semicolon)) {
                auto error = CompilerError(ErrorCode::MissingSemicolon, beforeRecover->getStart());
                addError(std::move(error));
            }
            it += 1;
        }
    } else {
        COMPILER_ASSERT(false, "unreachable");
    }

    modules.back().structDeclarations.emplace_back(std::move(decl));
}

std::vector<ConstraintDeclaration> Parser::genericConstraintListRule(TokenTreeIterator &it,
                                                                     const RecoverPredicate &recoverPredicate) {
    std::vector<ConstraintDeclaration> result;
    while (it && it->isToken(TokenType::Where)) {
        auto constraint = genericConstraintRule(it);
        if (constraint) {
            result.emplace_back(std::move(*constraint));
        }
        auto beforeRecover = it;
        recoverUntil(it, [&recoverPredicate](const TokenTreeNode &node) {
            return node.isToken(TokenType::Where) || recoverPredicate(node);
        });
        if (beforeRecover != it) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
    }
    return result;
}


std::optional<PropertyDeclaration> Parser::propertyDeclarationRule(TokenTreeIterator &it) {
    auto beforeRecover = it;
    recoverUntil(it, [](const TokenTreeNode &node) {
        return node.isModifier() || node.isToken(TokenType::Identifier) || node.isToken(TokenType::Semicolon);
    });
    if (beforeRecover != it) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (it.isEnd()) {
        //BEAN
        return std::nullopt;
    }
    if (it->isToken(TokenType::Semicolon)) {
        if (beforeRecover == it) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, it->getStart());
            addError(std::move(error));
        }
        it += 1;
        return std::nullopt;
    }

    auto modifiers = modifierRule(it, [](const auto &) { return true; });
    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});

    beforeRecover = it;
    recoverUntil(it, {TokenType::Identifier, TokenType::Semicolon});
    if (beforeRecover != it) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (it.isEnd()) {
        return std::nullopt;
    }
    if (it->isToken(TokenType::Semicolon)) {
        if (beforeRecover == it) {
            auto error = CompilerError(ErrorCode::MissingPropertyName, it->getStart());
            addError(std::move(error));
        }
        it += 1;
        return std::nullopt;
    }
    if (!it->isToken(TokenType::Identifier)) {
        auto error = CompilerError(ErrorCode::MissingPropertyName, it->getStart());
        addError(std::move(error));
        return std::nullopt;
    }

    PropertyDeclaration prop(identifierRule(it));
    if (!modifiers.empty()) {
        prop.startPos = modifiers.front().start;
    } else {
        prop.startPos = prop.name.start();
    }
    prop.isMutable = containsModifier(modifiers, TokenType::Mut);
    prop.isPublic = containsModifier(modifiers, TokenType::Pub);

    prop.endPos = prop.name.start();

    beforeRecover = it;
    recoverUntil(it, [&](const TokenTreeNode &node) {
        return node.isSignatureStarter() || node.isToken(TokenType::Colon) || node.isToken(TokenType::Semicolon);
    });
    if (beforeRecover != it) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingPropertyType, it->getStart());
        addError(std::move(error));
        return std::move(prop);
    }
    if (it->isToken(TokenType::Colon)) {
        prop.endPos = it->getEnd();
        it += 1;
        beforeRecover = it;
        recoverUntil(it, [&](const TokenTreeNode &node) {
            return node.isSignatureStarter() || node.isToken(TokenType::Semicolon);
        });
        if (beforeRecover != it) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            addError(std::move(error));
        }
        if (it.isEnd()) {
            auto error = CompilerError(ErrorCode::MissingPropertyType, it->getStart());
            addError(std::move(error));
            return std::nullopt;
        }
    }
    if (!it->isSignatureStarter()) {
        auto error = CompilerError(ErrorCode::MissingPropertyType, it->getStart());
        addError(std::move(error));
        return std::move(prop);
    }

    auto signature = signatureRule(it);

    prop.endPos = signature->end();
    prop.type = std::move(signature);

    beforeRecover = it;
    recoverUntil(it, [&](const TokenTreeNode &node) {
        return node.isModifier() || node.isToken(TokenType::Identifier) || node.isToken(TokenType::Semicolon);
    });

    if (beforeRecover != it) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
    if (it.isEnd() || !it->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd());
        addError(std::move(error));
    }
    prop.endPos = it->getEnd();
    it += 1;
    return std::move(prop);
}

void Parser::functionRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    const auto fnToken = consumeToken(it, TokenType::Fn);
    auto &decl = modules.back().functionDeclarations.emplace_back();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.front().start;
    } else {
        decl.startPos = fnToken.start;
    }
    decl.endPos = fnToken.end;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter()
               || n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenAngle)
               || n.isTokenTree(TokenType::OpenParen)
               || n.isToken(TokenType::DashArrow)
               || n.isToken(TokenType::Where)
               || n.isToken(TokenType::EqualArrow)
               || n.isTokenTree(TokenType::OpenCurly);
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, decl.start()));
        return;
    }

    if (it->isToken(TokenType::Identifier)) {
        decl.name = identifierRule(it);
        decl.endPos = decl.name->end();

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(TokenType::OpenAngle)
                   || n.isTokenTree(TokenType::OpenParen)
                   || n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return;
        }
    } else {
        addError(CompilerError(ErrorCode::MissingDeclarationName, decl.start()));
    }

    if (it->isTokenTree(TokenType::OpenAngle)) {
        decl.genericParams = identifierListRule(consumeTokenTree(it, TokenType::OpenAngle));

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(TokenType::OpenParen)
                   || n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return;
        }
    }

    if (it->isTokenTree(TokenType::OpenParen)) {
        decl.parameters = parameterListRule(consumeTokenTree(it, TokenType::OpenParen));

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return;
        }
    } else {
        addError(CompilerError(ErrorCode::MissingMethodParams, decl.start()));
    }

    if (it->isToken(TokenType::DashArrow)) {
        consumeToken(it, TokenType::DashArrow);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isSignatureStarter()
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return;
        }

        if (it->isSignatureStarter()) {
            decl.returnType = signatureRule(it);
        } else {
            addError(CompilerError(ErrorCode::MissingMethodReturnType, decl.start()));
        }
    }

    if (it->isToken(TokenType::Where)) {
        decl.genericConstraints = genericConstraintListRule(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return;
        }
    }

    if (it->isToken(TokenType::EqualArrow)) {
        consumeToken(it, TokenType::EqualArrow);
        if (it->isToken(TokenType::Semicolon)) {
            consumeToken(it, TokenType::Semicolon);
        } else {
            addError(CompilerError(ErrorCode::MissingSemicolon, decl.start()));
        }
    } else if (it->isTokenTree(TokenType::OpenCurly)) {
        consumeTokenTree(it, TokenType::OpenCurly);
    } else {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        return;
    }
}

void Parser::aliasRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    COMPILER_ASSERT(it->isToken(TokenType::Alias),
                    "aliasRule called with non-alias starting token");

    auto decl = AliasDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = it->getStart();
    }
    decl.endPos = it->getEnd();
    it += 1;

    validateModifiers(modifiers, {TokenType::Pub});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    auto beforeRecover = it;
    recoverUntil(it, [](const TokenTreeNode &node) {
        return node.isTopLevelStarter() || node.isToken(TokenType::Identifier);
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
        return;
    }

    if (beforeRecover != it) {
        addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
    }

    decl.name = identifierRule(it);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected alias name", *it);
        addError(std::move(error));
        return;
    }
    decl.endPos = decl.name->end();

    beforeRecover = it;
    recoverUntil(it, [](const TokenTreeNode &node) {
        return node.isTopLevelStarter() || node.isSignatureStarter() || node.isToken(TokenType::Equals) || node.
               isTokenTree(TokenType::OpenAngle);
    });

    if (it.isEnd() || it->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingAliasType, decl.start());
        addError(std::move(error));
        modules.back().aliasDeclarations.emplace_back(std::move(decl));
        return;
    }
    if (it != beforeRecover) {
        addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
    }
    if (const auto tree = tryConsumeTokenTree(it, TokenType::OpenAngle)) {
        decl.genericParams = identifierListRule(**tree);

        beforeRecover = it;
        recoverUntil(it, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isSignatureStarter() || node.isToken(TokenType::Equals);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            auto error = CompilerError(ErrorCode::MissingAliasType, decl.start());
            addError(std::move(error));
            modules.back().aliasDeclarations.emplace_back(std::move(decl));
            return;
        }
        if (it != beforeRecover) {
            addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
    }
    if (it->isToken(TokenType::Equals)) {
        it += 1;
        beforeRecover = it;
        recoverUntil(it, [](const TokenTreeNode &node) {
            return node.isTopLevelStarter() || node.isSignatureStarter();
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            auto error = CompilerError(ErrorCode::MissingAliasType, decl.start());
            addError(std::move(error));
            modules.back().aliasDeclarations.emplace_back(std::move(decl));
            return;
        }
        if (it != beforeRecover) {
            addError(CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
    } else {
        auto error = CompilerError(ErrorCode::MissingEquals, decl.start());
        addError(std::move(error));
    }


    auto signature = signatureRule(it);
    if (!signature) {
        modules.back().aliasDeclarations.emplace_back(std::move(decl));
        return;
    }
    decl.signature = std::move(signature);
    decl.endPos = (*decl.signature)->end();


    decl.genericConstraints = genericConstraintListRule(it, [](const TokenTreeNode &node) {
        return node.isToken(TokenType::Semicolon)
               || node.isTopLevelStarter();
    });
    if (!decl.genericConstraints.empty()) {
        decl.endPos = decl.genericConstraints.back().end();
    }

    if (it.isEnd() || it->isTopLevelStarter()) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, (it - 1)->getEnd());
    } else {
        COMPILER_ASSERT(it->isToken(TokenType::Semicolon), "unexpected token type");
        it += 1;
    }

    modules.back().aliasDeclarations.emplace_back(std::move(decl));
}

void Parser::moduleVariableRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    COMPILER_ASSERT(it->isToken(TokenType::Let),
                    "moduleVariableRule called with non-let starting token");

    auto decl = ModuleVariableDeclaration();
    if (!modifiers.empty()) {
        decl.startPos = modifiers.begin()->start;
    } else {
        decl.startPos = it->getStart();
    }
    decl.endPos = it->getEnd();
    it += 1;

    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);
    decl.isMut = containsModifier(modifiers, TokenType::Mut);

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected variable name", *(it - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.name = identifierRule(it);
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::MissingDeclarationName, decl.startPos);
        error.addLabel("expected variable name", *it);
        addError(std::move(error));
    }
    decl.endPos = decl.name->end();

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, decl.startPos);
        error.addLabel("missing semicolon", *(it - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!it->isToken(TokenType::Colon)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.startPos);
        error.addLabel("expected colon: `:`", *it);
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.endPos = it->getEnd();
    it += 1;

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::MissingVariableType, decl.startPos);
        error.addLabel("expected a type signature", *(it - 1));
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    decl.type = signatureRule(it);

    if (!decl.type) {
        auto error = CompilerError(ErrorCode::MissingVariableType, decl.startPos);
        error.addLabel("expected a type signature", *it);
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (it.isEnd()) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, (it - 1)->getEnd());
        error.setNote("unexpected end of method declaration, expected `;`");
        addError(std::move(error));
        modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
        return;
    }

    if (!it->isToken(TokenType::Semicolon)) {
        auto error = CompilerError(ErrorCode::MissingSemicolon, decl.start());
        error.addLabel("Missing semicolon", *it);
        addError(std::move(error));
    } else {
        decl.endPos = it->getEnd();
        it += 1;
    }

    modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
}

void Parser::declarationRule(TokenTreeIterator &it) {
    COMPILER_ASSERT(it->isModifier() || it->isDeclaratorKeyword(),
                    "declarationRule called with non-declaration starting token");

    auto modifiers = modifierRule(it, [](const auto &) { return true; });

    const auto &currentTokenTree = *it;
    if (currentTokenTree.isTokenResult()) {
        const auto &tokenResult = currentTokenTree.getTokenResult();
        if (tokenResult.isError()) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, it->getStart());
            error.addLabel("expected a top level declaration", *it);
            addError(std::move(error));
            recoverTopLevel(it);
            return;
        }
        auto &token = tokenResult.get();

        if (token.type == TokenType::Enum) {
            enumRule(it, std::move(modifiers));
        } else if (token.type == TokenType::Interface) {
            interfaceRule(it, std::move(modifiers));
        } else if (token.type == TokenType::Struct) {
            structRule(it, std::move(modifiers));
        } else if (token.type == TokenType::Fn) {
            functionRule(it, std::move(modifiers));
        } else if (token.type == TokenType::Alias) {
            aliasRule(it, std::move(modifiers));
        } else if (token.type == TokenType::Let) {
            moduleVariableRule(it, std::move(modifiers));
        } else {
            auto error = CompilerError(ErrorCode::UnexpectedToken, it->getStart());
            error.addLabel("expected a top level declaration", *it);
            addError(std::move(error));
            recoverTopLevel(it);
        }
    } else {
        auto error = CompilerError(ErrorCode::UnexpectedToken, it->getStart());
        error.addLabel("expected a top level declaration", *it);
        addError(std::move(error));
        recoverTopLevel(it);
    }
}

void Parser::implRule(TokenTreeIterator &it) {
    const auto implToken = consumeToken(it, TokenType::Impl);
    auto &impl = modules.back().implBlocks.emplace_back();
    impl.startPos = implToken.start;

    recoverUntil(it, [](const auto &n) {
        return n.isTopLevelStarter()
               || n.isTokenTree(TokenType::OpenAngle)
               || n.isTypeSignatureStarter()
               || n.isToken(TokenType::For)
               || n.isToken(TokenType::Where)
               || n.isTokenTree(TokenType::OpenCurly);
    });
    if (it.isEnd() || it->isTopLevelStarter()) {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
        return;
    }

    // generics
    if (it->isTokenTree(TokenType::OpenAngle)) {
        impl.genericParams = identifierListRule(consumeTokenTree(it, TokenType::OpenAngle));

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTypeSignatureStarter()
                   || n.isToken(TokenType::For)
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
            return;
        }
    }

    // (interface on?) typename
    if (it->isTypeSignatureStarter()) {
        auto signature = typeSignatureRule(it);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(TokenType::On)
                   || n.isToken(TokenType::For)
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
            return;
        }

        if (tryConsumeToken(it, TokenType::On)) {
            impl.interfaceName = std::move(signature);
            impl.structName = typeSignatureRule(it);
            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(TokenType::For)
                       || n.isToken(TokenType::Where)
                       || n.isTokenTree(TokenType::OpenCurly);
            });
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
                return;
            }
        } else {
            impl.structName = std::move(signature);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, impl.start()));
        return;
    }

    // for interface?
    if (tryConsumeToken(it, TokenType::For)) {
        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTypeSignatureStarter()
                   || n.isToken(TokenType::Where)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
            return;
        }

        if (it->isTypeSignatureStarter()) {
            impl.forInterfaceName = typeSignatureRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(TokenType::Where)
                       || n.isTokenTree(TokenType::OpenCurly);
            });
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
                return;
            }
        } else {
            addError(CompilerError(ErrorCode::UnexpectedToken, impl.start()));
            return;
        }
    }

    // generic constraints
    if (it->isToken(TokenType::Where)) {
        impl.genericConstraints = genericConstraintListRule(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
            return;
        }
    }

    //  body
    if (it->isTokenTree(TokenType::OpenCurly)) {
        const auto &body = consumeTokenTree(it, TokenType::OpenCurly);

        auto bodyIt = TokenTreeIterator(body.tokens);
        while (!bodyIt.isEnd()) {
            recoverUntil(bodyIt, [](auto const &n) {
                return n.isModifier()
                       || n.isToken(TokenType::Identifier)
                       || n.isToken(TokenType::Tilde)
                       || n.isToken(TokenType::Fn)
                       || n.isToken(TokenType::Get)
                       || n.isToken(TokenType::Set);
            });

            if (bodyIt.isEnd()) {
                addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
                break;
            }

            std::vector<Token> modifiers;
            if (bodyIt->isModifier()) {
                modifiers = modifierRule(bodyIt, [](auto const &n) {
                    return n.isToken(TokenType::Identifier)
                           || n.isToken(TokenType::Tilde)
                           || n.isToken(TokenType::Fn)
                           || n.isToken(TokenType::Get)
                           || n.isToken(TokenType::Set);
                });
            }

            if (bodyIt.isEnd()) {
                addError(CompilerError(ErrorCode::UnexpectedEndOfInput, impl.start()));
                break;
            }

            if (bodyIt->isToken(TokenType::Identifier)) {
                impl.constructors.emplace_back(constructorRule(bodyIt, modifiers));
            } else if (bodyIt->isToken(TokenType::Tilde)) {
                impl.destructors.emplace_back(destructorRule(bodyIt, modifiers));
            } else if (bodyIt->isToken(TokenType::Fn)) {
                impl.methods.emplace_back(methodRule(bodyIt, modifiers));
            } else if (bodyIt->isToken(TokenType::Set)) {
                impl.getters.emplace_back(implGetterRule(bodyIt, modifiers));
            } else if (bodyIt->isToken(TokenType::Get)) {
                impl.setters.emplace_back(implSetterRule(bodyIt, modifiers));
            }
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, impl.start()));
    }
}

ConstructorDeclaration Parser::constructorRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    validateModifiers(modifiers, {TokenType::Pub});
    ConstructorDeclaration decl;
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    decl.name = identifierRule(it);

    recoverUntil(it, [](const auto &n) {
        return n.isTokenTree(TokenType::OpenParen)
               || n.isToken(TokenType::Colon)
               || n.isTokenTree(TokenType::OpenCurly);
    });
    if (it.isEnd()) {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        return std::move(decl);
    }

    if (it->isTokenTree(TokenType::OpenParen)) {
        decl.parameters = parameterListRule(consumeTokenTree(it, TokenType::OpenParen));

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::Colon)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (tryConsumeToken(it, TokenType::Colon)) {
        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::Identifier)
                   || n.isTokenTree(TokenType::OpenParen)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }

        if (it->isToken(TokenType::Identifier)) {
            decl.otherName = identifierRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(TokenType::OpenParen)
                       || n.isTokenTree(TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
                return std::move(decl);
            }
        } else {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        }

        if (it->isTokenTree(TokenType::OpenParen)) {
            consumeTokenTree(it, TokenType::OpenParen);
            //TODO: expressions

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
                return std::move(decl);
            }
        } else {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        }

        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    }

    if (it->isTokenTree(TokenType::OpenCurly)) {
        consumeTokenTree(it, TokenType::OpenCurly);
        //TODO: expressions
    } else {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
    }

    return std::move(decl);
}

DestructureDeclaration Parser::destructorRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    validateModifiers(modifiers, {});
    DestructureDeclaration decl;

    consumeToken(it, TokenType::Tilde);

    recoverUntil(it, [](const auto &n) {
        return n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenParen)
               || n.isTokenTree(TokenType::OpenCurly);
    });
    if (it.isEnd()) {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        return std::move(decl);
    }

    if (it->isToken(TokenType::Identifier)) {
        decl.name = identifierRule(it);

        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(TokenType::OpenParen)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (tryConsumeTokenTree(it, TokenType::OpenParen)) {
        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (it->isTokenTree(TokenType::OpenCurly)) {
        consumeTokenTree(it, TokenType::OpenCurly);
        //TODO: statements!
    }

    return std::move(decl);
}

ImplMethod Parser::methodRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut, TokenType::Static});
    ImplMethod decl;
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);
    decl.isMut = containsModifier(modifiers, TokenType::Mut);
    decl.isStatic = containsModifier(modifiers, TokenType::Static);

    consumeToken(it, TokenType::Fn);

    recoverUntil(it, [](const auto &n) {
        return n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenAngle)
               || n.isTokenTree(TokenType::OpenParen)
               || n.isToken(TokenType::DashArrow)
               || n.isToken(TokenType::Where)
               || n.isToken(TokenType::EqualArrow)
               || n.isTokenTree(TokenType::OpenCurly);
    });
    if (it.isEnd()) {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        return std::move(decl);
    }

    if (it->isToken(TokenType::Identifier)) {
        decl.name = identifierRule(it);

        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(TokenType::OpenAngle)
                   || n.isTokenTree(TokenType::OpenParen)
                   || n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (it->isTokenTree(TokenType::OpenAngle)) {
        decl.genericParams = identifierListRule(consumeTokenTree(it, TokenType::OpenAngle));

        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(TokenType::OpenParen)
                   || n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    }

    if (it->isTokenTree(TokenType::OpenParen)) {
        decl.parameters = parameterListRule(consumeTokenTree(it, TokenType::OpenParen));

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (tryConsumeToken(it, TokenType::DashArrow)) {
        recoverUntil(it, [](const auto &n) {
            return n.isSignatureStarter()
                   || n.isToken(TokenType::Mut)
                   || n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        if (it->isSignatureStarter() || it->isToken(TokenType::Mut)) {
            decl.returnType = returnTypeRule(it);
        } else {
            addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
        }

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::Where)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    }

    if (it->isToken(TokenType::Where)) {
        decl.genericConstraints = genericConstraintListRule(it, [](const auto &n) {
            return n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    }

    if (it->isToken(TokenType::EqualArrow)) {
        consumeToken(it, TokenType::EqualArrow);
        //TODO: expressions
        consumeToken(it, TokenType::Semicolon);
    } else if (it->isTokenTree(TokenType::OpenCurly)) {
        consumeTokenTree(it, TokenType::OpenCurly);
        //TODO: statements
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    return std::move(decl);
}

ImplGetter Parser::implGetterRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    validateModifiers(modifiers, {TokenType::Pub, TokenType::Mut});
    ImplGetter decl;
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);
    decl.isMut = containsModifier(modifiers, TokenType::Mut);

    consumeToken(it, TokenType::Get);

    recoverUntil(it, [](const auto &n) {
        return n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenParen)
               || n.isToken(TokenType::DashArrow)
               || n.isToken(TokenType::EqualArrow)
               || n.isTokenTree(TokenType::OpenCurly);
    });
    if (it.isEnd()) {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        return std::move(decl);
    }

    if (it->isToken(TokenType::Identifier)) {
        decl.name = identifierRule(it);

        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(TokenType::OpenParen)
                   || n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (it->isTokenTree(TokenType::OpenParen)) {
        consumeTokenTree(it, TokenType::OpenParen);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::DashArrow)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (it->isToken(TokenType::DashArrow)) {
        consumeToken(it, TokenType::DashArrow);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::Mut) || n.isSignatureStarter()
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }

        if (it->isToken(TokenType::Mut) || it->isSignatureStarter()) {
            decl.returnType = returnTypeRule(it);
        } else {
            addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
        }

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (it->isToken(TokenType::EqualArrow)) {
        consumeToken(it, TokenType::EqualArrow);
        consumeToken(it, TokenType::Semicolon);
        //TODO: expressions
    } else if (it->isTokenTree(TokenType::OpenCurly)) {
        consumeTokenTree(it, TokenType::OpenCurly);
        //TODO: statements
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    return std::move(decl);
}

ImplSetter Parser::implSetterRule(TokenTreeIterator &it, std::vector<Token> modifiers) {
    validateModifiers(modifiers, {TokenType::Pub});
    ImplSetter decl;
    decl.isPublic = containsModifier(modifiers, TokenType::Pub);

    consumeToken(it, TokenType::Set);

    recoverUntil(it, [](const auto &n) {
        return n.isToken(TokenType::Identifier)
               || n.isTokenTree(TokenType::OpenParen)
               || n.isToken(TokenType::EqualArrow)
               || n.isTokenTree(TokenType::OpenCurly);
    });
    if (it.isEnd()) {
        addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
        return std::move(decl);
    }

    if (it->isToken(TokenType::Identifier)) {
        decl.name = identifierRule(it);

        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(TokenType::OpenParen)
                   || n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (it->isTokenTree(TokenType::OpenParen)) {
        auto params = parameterListRule(consumeTokenTree(it, TokenType::OpenParen));
        if(params.size() != 1) {
            addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
        }
        decl.parameter = std::move(params[0]);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(TokenType::EqualArrow)
                   || n.isTokenTree(TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::UnexpectedEndOfInput, decl.start()));
            return std::move(decl);
        }
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    if (it->isToken(TokenType::EqualArrow)) {
        consumeToken(it, TokenType::EqualArrow);
        consumeToken(it, TokenType::Semicolon);
        //TODO: expressions
    } else if (it->isTokenTree(TokenType::OpenCurly)) {
        consumeTokenTree(it, TokenType::OpenCurly);
        //TODO: statements
    } else {
        addError(CompilerError(ErrorCode::UnexpectedToken, decl.start()));
    }

    return std::move(decl);
}

Path Parser::pathRule(TokenTreeIterator &it, const bool allowTrailing) {
    Path path{};

    if ((path.rooted = tryConsumeToken(it, TokenType::PathSeparator))) {
        recoverUntil(it, TokenType::Identifier);
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::EmptyPath, path.rooted->start));
            return path;
        }
    }

    path.parts.push_back(identifierRule(it));

    while (it && it->isToken(TokenType::PathSeparator)) {
        const auto fallback = it;
        auto trailer = consumeToken(it, TokenType::PathSeparator);
        if (it.isEnd() || !it->isToken(TokenType::Identifier)) {
            if (allowTrailing) {
                path.trailer = trailer;
            } else {
                it = fallback;
            }
            break;
        }

        path.parts.push_back(identifierRule(it));
    }
    return path;
}

std::optional<ConstraintDeclaration> Parser::genericConstraintRule(TokenTreeIterator &it) {
    COMPILER_ASSERT(it && it->isToken(TokenType::Where),
                    "constraint rule called but start token is not a `where`");
    it += 1;
    auto decl = ConstraintDeclaration();

    // TODO: error recovery?

    decl.name = std::move(identifierRule(it));
    if (!decl.name) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.startPos);
        error.addLabel("expected generic parameter name", *it);
        addError(std::move(error));
        return std::nullopt;
    }
    decl.startPos = decl.name->start();
    decl.endPos = decl.name->end();

    // TODO: error recovery and end checking
    if (!it->isToken(TokenType::Colon)) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, decl.startPos);
        error.addLabel("expected `:`", *it);
        addError(std::move(error));
    } else {
        it += 1;
    }

    while (it) {
        if (auto c1 = interfaceConstraintRule(it)) {
            decl.constraints.emplace_back(std::make_unique<InterfaceConstraint>(std::move(*c1)));
        } /* else if (auto c2 = defaultConstraintRule(it)) {
            decl.constraints.push_back(std::move(c2));
        }*/
        else if (it->isConstraintBreakout()) {
            break;
        } else {
            auto error = CompilerError(ErrorCode::InvalidGenericConstraint, decl.startPos);
            error.addLabel("expected generic constraint", *it);
            addError(std::move(error));
            recoverUntil(it, [](const TokenTreeNode &node) {
                return node.isConstraintBreakout() || node.isToken(TokenType::Comma);
            });
        }

        if (it && it->isToken(TokenType::Comma)) {
            it += 1;
        } else {
            break;
        }
    }


    decl.endPos = it->getEnd();
    return decl;
}

std::optional<InterfaceConstraint> Parser::interfaceConstraintRule(TokenTreeIterator &it) {
    InterfaceConstraint constraint;
    constraint.typeSignature = typeSignatureRule(it);
    return constraint;
}

Identifier Parser::identifierRule(TokenTreeIterator &it) const {
    return Identifier::make(consumeToken(it, TokenType::Identifier), source);
}

std::unique_ptr<SignatureBase> Parser::signatureRule(TokenTreeIterator &it) {
    COMPILER_ASSERT(it && it->isSignatureStarter(), "signatureRule called with non signature starter");

    if (it->isPathStarter()) {
        return std::make_unique<TypeSignature>(typeSignatureRule(it));
    }
    if (it->isToken(TokenType::Fn)) {
        return std::make_unique<FunctionSignature>(functionSignatureRule(it));
    }
    if (it->isTokenTree(TokenType::OpenParen)) {
        return std::make_unique<TupleSignature>(tupleSignatureRule(it));
    }

    COMPILER_ASSERT(false, "signature rule called with wrong input");
}

TypeSignature Parser::typeSignatureRule(TokenTreeIterator &it) {
    auto decl = TypeSignature();

    decl.path = std::move(pathRule(it, false));
    decl.startPos = decl.path.start();
    decl.endPos = decl.path.end();

    if (it.isEnd()) {
        return decl;
    }

    if (const auto tree = tryConsumeTokenTree(it, TokenType::OpenAngle)) {
        decl.genericArguments = std::move(signatureListRule(**tree));
        decl.endPos = (*tree)->right.getEnd();
    }

    return decl;
}

FunctionSignature Parser::functionSignatureRule(TokenTreeIterator &it) {
    const auto fnToken = consumeToken(it, TokenType::Fn);
    auto decl = FunctionSignature();
    decl.startPos = fnToken.start;
    decl.endPos = fnToken.end;

    if (it.isEnd()) {
        addError(CompilerError(ErrorCode::FnSignatureMissingParams, decl.start()));
        return decl;
    }

    if (const auto parens = tryConsumeTokenTree(it, TokenType::OpenParen)) {
        decl.parameterTypes = signatureListRule(**parens);
        decl.endPos = (*parens)->right.getEnd();

        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::FnSignatureMissingParams, decl.start()));
            return decl;
        }
    } else {
        addError(CompilerError(ErrorCode::MissingMethodReturnType, it->getStart()));
    }

    if (const auto dashArrow = tryConsumeToken(it, TokenType::DashArrow)) {
        decl.endPos = dashArrow->end;

        if (it->isSignatureStarter()) {
            decl.returnType = returnTypeRule(it);
            decl.endPos = (*decl.returnType).end();
        } else {
            addError(CompilerError(ErrorCode::MissingMethodReturnType, dashArrow->start));
        }
    }

    return decl;
}

TupleSignature Parser::tupleSignatureRule(TokenTreeIterator &it) {
    auto decl = TupleSignature();

    auto const &list = consumeTokenTree(it, TokenType::OpenParen);
    decl.startPos = list.left.start;
    decl.endPos = list.right.getEnd();

    decl.types = signatureListRule(list);

    return decl;
}

ReturnType Parser::returnTypeRule(TokenTreeIterator &it) {
    ReturnType result;

    auto modifiers = modifierRule(it, [](const auto &n) {
        return true;
    });
    validateModifiers(modifiers, {TokenType::Mut});
    result.isMut = containsModifier(modifiers, TokenType::Mut);

    if (!modifiers.empty()) {
        result.startPos = modifiers.front().start;
        result.endPos = modifiers.back().end;
    }

    if (it.isEnd() || !it->isSignatureStarter()) {
        addError(CompilerError(ErrorCode::MissingMethodReturnType, it->getStart()));
        return result;
    }

    result.type = signatureRule(it);
    if (modifiers.empty()) {
        result.startPos = result.type->start();
    }
    result.endPos = result.type->end();

    return std::move(result);
}

std::vector<Identifier> Parser::identifierListRule(const TokenTree &list) {
    std::vector<Identifier> result;

    auto it = TokenTreeIterator(list.tokens);

    while (it) {
        recoverUntil(it, {TokenType::Identifier, TokenType::Comma});
        if (it.isEnd()) {
            break;
        }

        if (it->isToken(TokenType::Identifier)) {
            result.emplace_back(identifierRule(it));
            recoverUntil(it, {TokenType::Identifier, TokenType::Comma});
            if (it.isEnd()) {
                break;
            }
        } else {
            addError(CompilerError(ErrorCode::UnexpectedToken, it->getStart()));
            it += 1;
            continue;
        }

        if (!tryConsumeToken(it, TokenType::Comma)) {
            addError(CompilerError(ErrorCode::MissingComma, it->getStart()));
        }
    }

    return result;
}

std::vector<Parameter> Parser::parameterListRule(const TokenTree &list) {
    std::vector<Parameter> result;

    auto it = TokenTreeIterator(list.tokens);
    while (it) {
        auto startPos = it->getStart();
        auto modifiers = modifierRule(it, [](const auto &) { return true; });
        validateModifiers(modifiers, {TokenType::Mut, TokenType::Ref});

        auto beforeRecover = it;
        recoverUntil(it, [](const TokenTreeNode &node) {
            return node.isToken(TokenType::Identifier) || node.isToken(TokenType::Comma);
        });
        if (beforeRecover != it) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            error.addLabel("unexpected token", *beforeRecover);
            addError(std::move(error));
        }
        if (it.isEnd() || !it->isToken(TokenType::Identifier)) {
            addError(CompilerError(ErrorCode::ParameterNameMissing, (it - 1)->getEnd()));
            continue;
        }

        auto identifier = identifierRule(it);

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

        beforeRecover = it;
        recoverUntil(it, [](const TokenTreeNode &n) {
            return n.isSignatureStarter() || n.isToken(TokenType::Colon) || n.isToken(TokenType::Comma);
        });
        if (beforeRecover != it) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            error.addLabel("unexpected token", *beforeRecover);
            addError(std::move(error));
        }
        if (it.isEnd()) {
            addError(CompilerError(ErrorCode::ParameterTypeMissing, (it - 1)->getEnd()));
            continue;
        }
        if (it->isToken(TokenType::Comma)) {
            addError(CompilerError(ErrorCode::ParameterTypeMissing, it->getStart()));
            param.endPos = it->getEnd();
            it += 1;
            continue;
        }

        if (!it->isToken(TokenType::Colon)) {
            auto error = CompilerError(ErrorCode::MissingColon, it->getStart());
            error.addLabel("expected `:` to separate the parameter name and type", *it);
            addError(std::move(error));
        } else {
            param.endPos = it->getEnd();
            it += 1;
            beforeRecover = it;
            recoverUntil(it, [](const TokenTreeNode &node) {
                return node.isSignatureStarter() || node.isToken(TokenType::Comma);
            });
            if (beforeRecover != it) {
                auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
                error.addLabel("unexpected token", *beforeRecover);
                addError(std::move(error));
            }
            if (it.isEnd() || it->isToken(TokenType::Comma)) {
                addError(CompilerError(ErrorCode::ParameterTypeMissing, (it - 1)->getEnd()));
                continue;
            }
        }
        COMPILER_ASSERT(it && it->isSignatureStarter(),
                        "it must be a signature starter at this point");


        param.type = signatureRule(it);
        if (param.type) {
            param.endPos = (*param.type)->end();
        }
        beforeRecover = it;
        recoverUntil(it, TokenType::Comma); // TODO: move unexpected token error into recoverUntil?
        if (it != beforeRecover) {
            auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
            error.addLabel("unexpected token", *beforeRecover);
            addError(std::move(error));
        }

        DEBUG_ASSERT(it.isEnd() || it->isToken(TokenType::Comma), "expeced end or comma");

        it += 1;
    }

    return std::move(result);
}

std::vector<std::unique_ptr<SignatureBase> > Parser::signatureListRule(const TokenTree &list) {
    std::vector<std::unique_ptr<SignatureBase> > result;

    auto it = TokenTreeIterator(list.tokens);
    const auto end = list.tokens.end();

    while (it) {
        if (it->isSignatureStarter()) {
            result.emplace_back(signatureRule(it));
        } else {
            auto error = CompilerError(ErrorCode::InvalidSignature, list.left);
            error.addLabel("expected type, tuple or function signature", *it);
            addError(std::move(error));
            recoverUntil(it, [](const TokenTreeNode &node) {
                return node.isSignatureStarter() || node.isToken(TokenType::Comma);
            });
        }

        if (it) {
            if (it->isToken(TokenType::Comma)) {
                it += 1;
            } else {
                auto error = CompilerError(ErrorCode::MissingComma, list.left);
                error.addLabel("expected a comma", *it);
                addError(std::move(error));
            }
        }
    }

    return std::move(result);
}

void Parser::recoverTopLevel(TokenTreeIterator &it) {
    while (it) {
        recoverUntil(it, [](const TokenTreeNode &node) {
            return node.isToken(TokenType::Semicolon)
                   || node.isTopLevelStarter()
                   || node.isModifier();
        });
        if (!tryConsumeToken(it, TokenType::Semicolon)) {
            break;
        }
    }
}

void Parser::recoverUntil(TokenTreeIterator &it, TokenType type) {
    recoverUntil(it, [type](const TokenTreeNode &node) {
        return node.isToken(type);
    });
}

void Parser::recoverUntil(TokenTreeIterator &it, std::vector<TokenType> oneOf) {
    recoverUntil(it, [&oneOf](const TokenTreeNode &node) {
        return node.isToken() && std::ranges::find(oneOf, node.getToken().type) != oneOf.end();
    });
}

void Parser::recoverUntil(TokenTreeIterator &it, const RecoverPredicate &predicate) {
    auto beforeRecover = it;
    while (it && !predicate(*it)) {
        it += 1;
    }

    if (beforeRecover != it) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        addError(std::move(error));
    }
}

void Parser::recoverUntil(TokenTreeIterator &it, const RecoverPredicate &predicate,
                          ErrorContext &errCtx) {
    const auto beforeRecover = it;
    while (it && !predicate(*it)) {
        it += 1;
    }

    if (beforeRecover != it) {
        auto error = CompilerError(ErrorCode::UnexpectedToken, beforeRecover->getStart());
        errCtx.addError(std::move(error));
    }
}

Token Parser::consumeToken(TokenTreeIterator &it, const TokenType type) {
    COMPILER_ASSERT(it, std::format("trying to consume {} on empty iterator", TokenTypeName(type)));
    COMPILER_ASSERT(it->isToken(type),
                    std::format("trying to consume {} but got {}", TokenTypeName(type), it->debugString()));

    auto const token = it->getToken();
    it += 1;
    return token;
}

std::optional<Token> Parser::tryConsumeToken(TokenTreeIterator &it, const TokenType type) {
    if (it.isEnd() || !it->isToken(type)) {
        return std::nullopt;
    }

    auto token = it->getToken();
    it += 1;
    return token;
}

auto Parser::consumeTokenTree(TokenTreeIterator &it, TokenType type) -> const TokenTree & {
    COMPILER_ASSERT(it,
                    std::format("trying to consume TokenTree[{}] on empty iterator", TokenTypeName(type)));
    COMPILER_ASSERT(it->isTokenTree(type),
                    std::format("trying to consume TokenTree[{}] but got {}", TokenTypeName(type), it->debugString()
                    ));

    auto const &tree = it->getTokenTree();

    if (tree.right.isError()) {
        auto error = CompilerError(ErrorCode::WrongCloser, tree.right.getOrErrorToken());
        error.setNote("unclosed list, expected: " + TokenTypeStringQuoted(tree.left.expectedClosing()));
        addError(std::move(error));
    }

    it += 1;
    return tree;
}

std::optional<const TokenTree *> Parser::tryConsumeTokenTree(TokenTreeIterator &it,
                                                             const TokenType type) {
    if (it.isEnd() || !it->isTokenTree(type)) {
        return std::nullopt;
    }

    auto const &tree = it->getTokenTree();

    if (tree.right.isError()) {
        auto error = CompilerError(ErrorCode::WrongCloser, tree.right.getOrErrorToken());
        error.setNote("unclosed list: expected " + TokenTypeStringQuoted(tree.left.expectedClosing()));
        addError(std::move(error));
    }

    it += 1;
    return &tree;
}

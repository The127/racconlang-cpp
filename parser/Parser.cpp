//
// Created by zoe on 07.09.24.
//

#include "Parser.h"

#include <algorithm>
#include <utility>
#include <ranges>

#include "sourceMap/Source.h"
#include "errors/InternalError.h"
#include "ast/AliasDeclaration.h"
#include "ast/ConstraintDeclaration.h"
#include "ast/ConstructorDeclaration.h"
#include "ast/DestructureDeclaration.h"
#include "ast/EnumDeclaration.h"
#include "ast/EnumMemberDeclaration.h"
#include "ast/UseMap.h"
#include "ast/FunctionDeclaration.h"
#include "ast/FunctionSignature.h"
#include "ast/InterfaceConstraint.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/InterfaceGetterDeclaration.h"
#include "ast/InterfaceMethodDeclaration.h"
#include "ast/InterfaceSetterDeclaration.h"
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
#include "ast/Parameter.h"
#include "ast/NamelessParameter.h"
#include "errors/ErrorContext.h"
#include "errors/ErrorCode.h"
#include "registry/ModuleRegistry.h"

namespace racc::parser {

    Parser::Parser(const std::shared_ptr<sourcemap::Source> &source)
            : source(source) {
        modules.emplace_back();
    }

    Parser::Parser(Parser &&) noexcept = default;

    Parser &Parser::operator=(Parser &&) noexcept = default;

    Parser::~Parser() = default;

    std::vector<ast::ModuleDeclaration> Parser::parse() {
        parseFile();

        auto useMap = std::make_shared<ast::UseMap>();
        for (const auto &node: uses) {
            COMPILER_ASSERT(!node.path.parts.empty(), "use path is empty");

            std::string path;
            for (auto &part: node.path.parts) {
                path += part.name;
                if (&part != &node.path.parts.back()) {
                    path += "::";
                }
            }

            if (node.names.empty()) {
                auto as = node.as.and_then([](const auto &i) -> std::optional<std::string> {
                    return std::string(i.name);
                }).value_or(std::string(node.path.parts.back().name));
                auto [_, success] = useMap->uses.emplace(as, path);
                COMPILER_ASSERT(success, "TODO: error handling");

            } else {
                COMPILER_ASSERT(!node.as, "unexpected as on use node");

                for (const auto &[nameIdent, asOpt]: node.names) {
                    auto as = std::string(asOpt.value_or(nameIdent).name);
                    auto name = std::string(nameIdent.name);
                    auto currentPath = path;
                    currentPath += "::";
                    currentPath += name;
                    auto [_, success] = useMap->uses.emplace(as, currentPath);
                    COMPILER_ASSERT(success, "TODO: error handling");
                }
            }
        }

        for (auto &module: modules) {
            module.uses = useMap;
        }

        return std::move(modules);
    }

    void Parser::addError(errors::CompilerError error) {
        source->addError(std::move(error));
    }

    void Parser::parseFile() {
        COMPILER_ASSERT(source->tokenTree, "Token tree was not set: " + source->fileName);

        const auto &tokenTree = *source->tokenTree;
        COMPILER_ASSERT(tokenTree.left.type == lexer::TokenType::Bof, "Token tree did not start with BOF: " + source->fileName);
        COMPILER_ASSERT(tokenTree.right.isToken(lexer::TokenType::Eof), "Token tree did not end with an EOF: " + source->fileName);

        auto it = TokenTreeIterator(tokenTree.tokens);

        while (it) {
            recoverTopLevel(it);
            if (it.isEnd()) {
                break;
            }
            if (it->isToken(lexer::TokenType::Use)) {
                useRule(it);
            } else if (it->isToken(lexer::TokenType::Mod)) {
                modRule(it);
            } else if (it->isModifier() || it->isDeclaratorKeyword()) {
                declarationRule(it);
            } else if (it->isToken(lexer::TokenType::Impl)) {
                implRule(it);
            } else {
                COMPILER_ASSERT(false, std::format("unhandled top level declaration, got {}", it->debugString()));
            }
        }
    }

    void Parser::useRule(TokenTreeIterator &it) {
        if (modules.size() > 1) {
            auto error = errors::CompilerError(errors::ErrorCode::UseAfterMod, it->getStart());
            error.setNote("uses need to be declared before any modules");
            addError(std::move(error));
        }

        const auto &useToken = consumeToken(it, lexer::TokenType::Use);

        recoverUntil(it, [](const auto &ttn) {
            return ttn.isTopLevelStarter()
                   || ttn.isPathStarter();
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::UseIsMissingPath, (it - 1)->getEnd()));
            return;
        }

        auto &use = uses.emplace_back();
        use.startPos = useToken.start;

        use.path = pathRule(it, true);
        use.endPos = use.path.end();

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter() || n.isToken(lexer::TokenType::Semicolon) || n.isToken(lexer::TokenType::As) ||
                   n.isTokenTree(lexer::TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
            return;
        }

        if (tryConsumeToken(it, lexer::TokenType::As)) {
            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter() || n.isToken(lexer::TokenType::Semicolon) || n.isToken(lexer::TokenType::Identifier);
            });
            if (it.isEnd() || it->isTopLevelStarter() || tryConsumeToken(it, lexer::TokenType::Semicolon)) {
                addError(errors::CompilerError(errors::ErrorCode::MissingAsName, (it - 1)->getEnd()));
                return;
            }

            use.as = identifierRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter() || n.isToken(lexer::TokenType::Semicolon);
            });
        }

        if (const auto semicolon = tryConsumeToken(it, lexer::TokenType::Semicolon)) {
            use.endPos = semicolon->end;
            return;
        }

        if (const auto tree = tryConsumeTokenTree(it, lexer::TokenType::OpenCurly)) {
            COMPILER_ASSERT(!use.as, "use.as cannot be set at this point");
            if (!use.path.isTrailing()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingPathSeparator, (*tree)->left.start));
            }

            auto listIt = TokenTreeIterator((*tree)->tokens);

            while (listIt) {
                recoverUntil(listIt, lexer::TokenType::Identifier);
                if (listIt.isEnd()) {
                    break;
                }

                auto &name = use.names.emplace_back(identifierRule(listIt), std::nullopt);

                recoverUntil(listIt, {lexer::TokenType::Identifier, lexer::TokenType::Comma, lexer::TokenType::As});

                if (tryConsumeToken(listIt, lexer::TokenType::As)) {
                    recoverUntil(listIt, {lexer::TokenType::Identifier, lexer::TokenType::Comma});
                    if (listIt.isEnd() || tryConsumeToken(listIt, lexer::TokenType::Comma)) {
                        addError(errors::CompilerError(errors::ErrorCode::MissingAsName, (listIt - 1)->getEnd()));
                        continue;
                    }
                    name.second = identifierRule(listIt);
                    recoverUntil(listIt, {lexer::TokenType::Identifier, lexer::TokenType::Comma});
                }

                if (listIt.isEnd() || tryConsumeToken(listIt, lexer::TokenType::Comma)) {
                    continue;
                }
                addError(errors::CompilerError(errors::ErrorCode::MissingComma, (listIt - 1)->getEnd()));
            }

            use.endPos = (*tree)->right.getEnd();
        }


        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter() || n.isToken(lexer::TokenType::Semicolon);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
            return;
        }
        const auto semicolon = consumeToken(it, lexer::TokenType::Semicolon);
        use.endPos = semicolon.end;
    }

    void Parser::modRule(TokenTreeIterator &it) {
        const auto modToken = consumeToken(it, lexer::TokenType::Mod);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter() || n.isPathStarter() || n.isToken(lexer::TokenType::Semicolon);
        });
        if (it.isEnd() || it->isTopLevelStarter() || it->isToken(lexer::TokenType::Semicolon)) {
            addError(errors::CompilerError(errors::ErrorCode::MissingModulePath, it->getStart()));
            tryConsumeToken(it, lexer::TokenType::Semicolon);
            return;
        }

        auto &mod = modules.emplace_back();
        mod.startPos = modToken.start;

        mod.path = pathRule(it, false);

        mod.endPos = mod.path.end();
        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter() || n.isToken(lexer::TokenType::Semicolon);
        });

        if (it.isEnd() || !it->isToken(lexer::TokenType::Semicolon)) {
            addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, it->getStart()));
        } else {
            mod.endPos = it->getEnd();
            consumeToken(it, lexer::TokenType::Semicolon);
        }
    }

    std::vector<lexer::Token> Parser::modifierRule(TokenTreeIterator &it,
                                                   const RecoverPredicate &recoverPredicate) {
        std::vector<lexer::Token> result;

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
                auto error = errors::CompilerError(errors::ErrorCode::DuplicateModifier, it->getStart());
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

    void Parser::validateModifiers(std::vector<lexer::Token> &modifiers, const std::vector<lexer::TokenType> &validTokenTypes) {
        // TODO: check that the order is correct (but still return all the valid modifiers)
        std::vector<lexer::Token> result;
        std::string note = "valid modifiers are: ";
        for (size_t i = 0; i < validTokenTypes.size(); ++i) {
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
                auto error = errors::CompilerError(errors::ErrorCode::InvalidModifier, modifiers[0]);
                error.addLabel("not a valid modifier here", modifier);
                error.setNote(note);
                addError(std::move(error));
            }
        }

        modifiers = std::move(result);
    }

    inline bool containsModifier(const std::vector<lexer::Token> &modifiers, const lexer::TokenType type) {
        return std::ranges::any_of(modifiers, [type](auto &m) { return m.type == type; });
    }

    void Parser::enumRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        auto &decl = modules.back().enumDeclarations.emplace_back();
        const auto enumToken = consumeToken(it, lexer::TokenType::Enum);

        if (!modifiers.empty()) {
            decl.startPos = modifiers.front().start;
        } else {
            decl.startPos = enumToken.start;
        }
        decl.endPos = enumToken.end;

        validateModifiers(modifiers, {lexer::TokenType::Pub});
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(lexer::TokenType::Identifier)
                   || n.isTokenTree(lexer::TokenType::OpenAngle)
                   || n.isToken(lexer::TokenType::Where)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
            return;
        }

        if (!it->isToken(lexer::TokenType::Identifier)) {
            addError(errors::CompilerError(errors::ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
        } else {
            decl.name = identifierRule(it);
            decl.endPos = decl.name->end();

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter() || n.isTokenTree(lexer::TokenType::OpenAngle) || n.isToken(lexer::TokenType::Where) || n.
                        isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
                return;
            }
        }


        if (const auto generics = tryConsumeTokenTree(it, lexer::TokenType::OpenAngle)) {
            decl.endPos = (*generics)->right.getEnd();
            decl.genericParams = identifierListRule(**generics);

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter() || n.isToken(lexer::TokenType::Where) || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
                return;
            }
        }


        if (it->isToken(lexer::TokenType::Where)) {
            decl.genericConstraints = genericConstraintListRule(it, [](const auto &node) {
                return node.isTopLevelStarter() || node.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (!decl.genericConstraints.empty()) {
                decl.endPos = decl.genericConstraints.back().end();
            }

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter() || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
                return;
            }
        }


        const auto &body = consumeTokenTree(it, lexer::TokenType::OpenCurly);
        decl.endPos = body.right.getEnd();

        if (body.right.isError()) {
            addError(errors::CompilerError(errors::ErrorCode::UnclosedEnumBody, body.right.getStart()));
        }

        auto bodyIt = TokenTreeIterator(body.tokens);

        while (bodyIt) {
            recoverUntil(bodyIt, lexer::TokenType::Identifier);
            if (bodyIt.isEnd()) {
                break;
            }
            decl.memberDeclarations.emplace_back(enumMemberRule(bodyIt));

            if (bodyIt.isEnd()) {
                break;
            }
            if (!tryConsumeToken(bodyIt, lexer::TokenType::Comma)) {
                addError(errors::CompilerError(errors::ErrorCode::MissingComma, (bodyIt - 1)->getEnd()));
            }
        }
    }

    ast::EnumMemberDeclaration Parser::enumMemberRule(TokenTreeIterator &it) {
        auto decl = ast::EnumMemberDeclaration(identifierRule(it));

        decl.startPos = decl.name.start();
        decl.endPos = decl.name.end();

        if (it.isEnd()) {
            return decl;
        }

        if (const auto &tree = tryConsumeTokenTree(it, lexer::TokenType::OpenParen)) {
            decl.endPos = (*tree)->right.getEnd();
            decl.values = signatureListRule(**tree);
        }
        return decl;
    }


    void Parser::interfaceRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        auto &decl = modules.back().interfaceDeclarations.emplace_back();
        auto interfaceToken = consumeToken(it, lexer::TokenType::Interface);

        if (!modifiers.empty()) {
            decl.startPos = modifiers.front().start;
        } else {
            decl.startPos = interfaceToken.start;
        }
        decl.endPos = interfaceToken.end;

        validateModifiers(modifiers, {lexer::TokenType::Pub});
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(lexer::TokenType::Identifier)
                   || n.isTokenTree(lexer::TokenType::OpenAngle)
                   || n.isToken(lexer::TokenType::Colon)
                   || n.isToken(lexer::TokenType::Where)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
            return;
        }

        if (!it->isToken(lexer::TokenType::Identifier)) {
            addError(errors::CompilerError(errors::ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
        } else {
            decl.name = identifierRule(it);
            decl.endPos = decl.name->end();

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTokenTree(lexer::TokenType::OpenAngle)
                       || n.isToken(lexer::TokenType::Colon)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingInterfaceBody, (it - 1)->getEnd()));
                return;
            }
        }


        if (const auto &generics = tryConsumeTokenTree(it, lexer::TokenType::OpenAngle)) {
            decl.endPos = (*generics)->right.getEnd();
            decl.genericParams = identifierListRule(**generics);

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(lexer::TokenType::Colon)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingInterfaceBody, (it - 1)->getEnd()));
                return;
            }
        }

        if (const auto &colon = tryConsumeToken(it, lexer::TokenType::Colon)) {
            decl.endPos = colon->end;

            while (it) {
                recoverUntil(it, [](const auto &n) {
                    return n.isTopLevelStarter()
                           || n.isPathStarter()
                           || n.isTokenTree(lexer::TokenType::Where)
                           || n.isTokenTree(lexer::TokenType::OpenCurly);
                });

                if (it.isEnd() || !it->isPathStarter()) {
                    break;
                }

                const auto &signature = decl.requiredInterfaces.emplace_back(typeSignatureRule(it));
                decl.endPos = signature.end();

                recoverUntil(it, [](const auto &n) {
                    return n.isTopLevelStarter()
                           || n.isToken(lexer::TokenType::Comma)
                           || n.isPathStarter()
                           || n.isToken(lexer::TokenType::Where)
                           || n.isTokenTree(lexer::TokenType::OpenCurly);
                });
                if (auto tok = tryConsumeToken(it, lexer::TokenType::Comma)) {
                    decl.endPos = tok->end;
                } else if (it->isPathStarter()) {
                    addError(errors::CompilerError(errors::ErrorCode::MissingComma, (it - 1)->getEnd()));
                } else {
                    break;
                }
            }

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(lexer::TokenType::Where)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingInterfaceBody, (it - 1)->getEnd()));
                return;
            }
        }


        while (it->isToken(lexer::TokenType::Where)) {
            decl.genericConstraints = genericConstraintListRule(it, [](const auto &node) {
                return node.isTopLevelStarter() || node.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (!decl.genericConstraints.empty()) {
                decl.endPos = decl.genericConstraints.back().end();
            }

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTokenTree(lexer::TokenType::OpenCurly)
                       || n.isToken(lexer::TokenType::Where);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingEnumBody, (it - 1)->getEnd()));
                return;
            }
        }


        const auto &bodyTree = consumeTokenTree(it, lexer::TokenType::OpenCurly);
        auto bodyIt = TokenTreeIterator(bodyTree.tokens);

        while (bodyIt) {
            auto originalIt = bodyIt;

            recoverUntil(bodyIt, [](const auto &n) {
                return n.isModifier()
                       || n.isToken(lexer::TokenType::Fn)
                       || n.isToken(lexer::TokenType::Get)
                       || n.isToken(lexer::TokenType::Set);
            });

            if (bodyIt.isEnd()) {
                break;
            }

            std::vector<lexer::Token> memberModifiers;
            if (bodyIt->isModifier()) {
                memberModifiers = modifierRule(bodyIt, [](const auto &n) {
                    return n.isToken(lexer::TokenType::Fn)
                           || n.isToken(lexer::TokenType::Get)
                           || n.isToken(lexer::TokenType::Set);
                });

                recoverUntil(bodyIt, [](const auto &n) {
                    return n.isToken(lexer::TokenType::Fn)
                           || n.isToken(lexer::TokenType::Get)
                           || n.isToken(lexer::TokenType::Set);
                });

                if (bodyIt.isEnd()) {
                    addError(errors::CompilerError(errors::ErrorCode::MissingInterfaceMember, originalIt->getStart()));
                    break;
                }
            }

            if (bodyIt->isToken(lexer::TokenType::Fn)) {
                decl.methods.emplace_back(interfaceMethodRule(bodyIt, std::move(memberModifiers)));
            } else if (bodyIt->isToken(lexer::TokenType::Get)) {
                decl.getters.emplace_back(interfaceGetterRule(bodyIt, std::move(memberModifiers)));
            } else if (bodyIt->isToken(lexer::TokenType::Set)) {
                decl.setters.emplace_back(interfaceSetterRule(bodyIt, std::move(memberModifiers)));
            } else {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, originalIt->getStart()));
            }
        }
    }


    ast::InterfaceMethodDeclaration Parser::interfaceMethodRule(TokenTreeIterator &it,
                                                                std::vector<lexer::Token> modifiers) {
        auto fnToken = consumeToken(it, lexer::TokenType::Fn);
        auto decl = ast::InterfaceMethodDeclaration();
        decl.startPos = fnToken.start;

        validateModifiers(modifiers, {lexer::TokenType::Pub, lexer::TokenType::Mut});
        if (!modifiers.empty()) {
            decl.startPos = modifiers.front().start;
            decl.isMut = containsModifier(modifiers, lexer::TokenType::Mut);
        }

        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingMethodName, (it - 1)->getEnd()));
            return decl;
        }

        decl.name = identifierRule(it);
        if (!decl.name) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingMethodName, decl.startPos);
            error.addLabel("expected method name", *it);
            addError(std::move(error));
            return decl;
        }
        decl.endPos = decl.name->end();

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, (it - 1)->getEnd());
            error.setNote("unexpected end of method declaration");
            addError(std::move(error));
            return decl;
        }

        if (!it->isTokenTree(lexer::TokenType::OpenAngle) && !it->isTokenTree(lexer::TokenType::OpenParen)) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start());
            error.addLabel("expected generic parameters or method parameters", *it);
            addError(std::move(error));
            recoverUntil(it, [](const auto &node) {
                return node.isTokenTree(lexer::TokenType::OpenParen)
                       || node.isTokenTree(lexer::TokenType::OpenAngle)
                       || node.isToken(lexer::TokenType::Semicolon);
            });
            if (it->isToken(lexer::TokenType::Semicolon)) {
                it += 1;
                return decl;
            }
            if (it.isEnd()) {
                return decl;
            }
        }

        if (const auto tree = tryConsumeTokenTree(it, lexer::TokenType::OpenAngle)) {
            decl.endPos = (*tree)->right.getEnd();
            decl.genericParams = identifierListRule(**tree);
        }

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, (it - 1)->getEnd());
            error.setNote("unexpected end of method declaration, expected method parameters");
            addError(std::move(error));
            return decl;
        }

        if (!it->isTokenTree(lexer::TokenType::OpenParen)) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start());
            error.addLabel("expected method parameters", *it);
            addError(std::move(error));
            return decl;
        }
        const auto &tree = consumeTokenTree(it, lexer::TokenType::OpenParen);
        decl.parameters = parameterListRule(tree);

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, (it - 1)->getEnd());
            error.setNote("unexpected end of method declaration, expected return type, generic constraints or `;`");
            addError(std::move(error));
            return decl;
        }

        decl.returnType = returnTypeRule(it);


        decl.genericConstraints = genericConstraintListRule(it, [](const auto &node) {
            return node.isToken(lexer::TokenType::Semicolon)
                   || node.isTopLevelStarter();
        });

        if (!decl.genericConstraints.empty()) {
            decl.endPos = decl.genericConstraints.back().end();
        }

        if (it.isEnd() || !it->isToken(lexer::TokenType::Semicolon)) {
            addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
            return decl;
        }

        decl.endPos = it->getEnd();
        it += 1;

        return decl;
    }

    ast::InterfaceGetterDeclaration Parser::interfaceGetterRule(TokenTreeIterator &it,
                                                                std::vector<lexer::Token> modifiers) {
        auto getToken = consumeToken(it, lexer::TokenType::Get);
        auto decl = ast::InterfaceGetterDeclaration();
        decl.startPos = getToken.start;

        validateModifiers(modifiers, {lexer::TokenType::Pub, lexer::TokenType::Mut});
        if (!modifiers.empty()) {
            decl.startPos = modifiers.front().start;
            decl.isMut = containsModifier(modifiers, lexer::TokenType::Mut);
        }

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingGetterName, (it - 1)->getEnd());
            error.setNote("unexpected end of getter declaration");
            addError(std::move(error));
            return decl;
        }

        decl.name = identifierRule(it);
        if (!decl.name) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingGetterName, decl.startPos);
            error.addLabel("expected getter name", *it);
            addError(std::move(error));
        }
        decl.endPos = decl.name->end();

        if (it.isEnd() || !it->isTokenTree(lexer::TokenType::OpenParen)) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingGetterParam, (it - 1)->getEnd());
            error.setNote("unexpected end of setter declaration, expected `(`");
            addError(std::move(error));
            return decl;
        }

        const auto &tree = consumeTokenTree(it, lexer::TokenType::OpenParen);
        decl.endPos = tree.right.getEnd();
        auto params = parameterListRule(tree);

        if (!params.empty()) {
            auto error = errors::CompilerError(errors::ErrorCode::TooManyGetterParams, params[0].start());
            error.setNote("getters cannot have parameters");
            addError(std::move(error));
        }

        decl.returnType = returnTypeRule(it);
        if (decl.returnType) {
            decl.endPos = decl.returnType->end();
        } else {
            auto error = errors::CompilerError(errors::ErrorCode::MissingReturnType, (it - 1)->getEnd());
            error.setNote("getters must have a return type");
            addError(std::move(error));
        }

        if (it->isToken(lexer::TokenType::Semicolon)) {
            decl.endPos = it->getEnd();
            it += 1;
        } else {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, it->getStart());
            error.setNote("expected a semicolon");
            addError(std::move(error));
        }

        return decl;
    }

    ast::InterfaceSetterDeclaration Parser::interfaceSetterRule(TokenTreeIterator &it,
                                                                std::vector<lexer::Token> modifiers) {
        auto setToken = consumeToken(it, lexer::TokenType::Set);
        auto decl = ast::InterfaceSetterDeclaration();
        decl.startPos = setToken.start;

        validateModifiers(modifiers, {lexer::TokenType::Pub});
        if (!modifiers.empty()) {
            decl.startPos = modifiers.front().start;
        }

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSetterName, (it - 1)->getEnd());
            error.setNote("unexpected end of setter declaration");
            addError(std::move(error));
            return decl;
        }

        recoverUntil(it, [](const auto &n) { return n.isToken(lexer::TokenType::Identifier) || n.isTokenTree(lexer::TokenType::OpenParen); });

        decl.name = identifierRule(it);
        if (!decl.name) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSetterName, decl.startPos);
            error.addLabel("expected setter name", *it);
            addError(std::move(error));
            return decl;
        }
        decl.endPos = decl.name->end();

        if (it.isEnd() || !it->isTokenTree(lexer::TokenType::OpenParen)) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSetterParam, (it - 1)->getEnd());
            error.setNote("unexpected end of setter declaration, expected `(`");
            addError(std::move(error));
            return decl;
        }

        const auto &tree = consumeTokenTree(it, lexer::TokenType::OpenParen);

        decl.endPos = tree.right.getEnd();
        auto params = parameterListRule(tree);

        if (params.size() > 1) {
            auto error = errors::CompilerError(errors::ErrorCode::TooManySetterParams, params[1].start());
            error.setNote("setters must have exactly 1 parameter");
            addError(std::move(error));
        }

        if (params.empty()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSetterParam, tree.right.getStart());
            error.setNote("setters must have exactly 1 parameter");
            addError(std::move(error));
        } else {
            decl.parameter = std::move(params[0]);
        }

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd());
            error.setNote("expected a semicolon");
            addError(std::move(error));
        }

        if (it->isToken(lexer::TokenType::Semicolon)) {
            decl.endPos = it->getEnd();
            it += 1;
        } else {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, it->getStart());
            error.setNote("expected a semicolon");
            addError(std::move(error));
        }

        return decl;
    }


    void Parser::structRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        COMPILER_ASSERT(it->isToken(lexer::TokenType::Struct), "structRule called with non-struct starting token");

        auto decl = ast::StructDeclaration();
        if (!modifiers.empty()) {
            decl.startPos = modifiers.begin()->start;
        } else {
            decl.startPos = it->getStart();
        }
        decl.endPos = it->getEnd();
        it += 1;

        validateModifiers(modifiers, {lexer::TokenType::Pub, lexer::TokenType::Value});
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);
        decl.isValue = containsModifier(modifiers, lexer::TokenType::Value);

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingDeclarationName, decl.startPos);
            error.addLabel("expected struct name", *it);
            addError(std::move(error));
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }

        decl.name = identifierRule(it);
        if (!decl.name) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingDeclarationName, decl.startPos);
            error.addLabel("expected struct name", *it);
            addError(std::move(error));
        }
        decl.endPos = decl.name->end();

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, decl.startPos);
            error.addLabel("expected semicolon", *it);
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }

        if (const auto tree = tryConsumeTokenTree(it, lexer::TokenType::OpenAngle)) {
            decl.endPos = (*tree)->right.getEnd();
            decl.genericParams = identifierListRule(**tree);
        }

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingStructBody, decl.startPos);
            error.addLabel("expected struct body", *it);
            addError(std::move(error));
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }

        auto beforeRecover = it;
        recoverUntil(it, [](const auto &node) {
            return node.isTokenTree(lexer::TokenType::OpenCurly)
                   || node.isTokenTree(lexer::TokenType::OpenParen)
                   || node.isToken(lexer::TokenType::Where)
                   || node.isTopLevelStarter();
        });
        if (beforeRecover != it) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
        if (it.isEnd() || it->isTopLevelStarter()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingStructBody, decl.startPos);
            error.addLabel("expected struct body", *(it - 1));
            addError(std::move(error));
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }


        decl.genericConstraints = genericConstraintListRule(it, [](const auto &node) {
            return node.isTokenTree(lexer::TokenType::OpenCurly)
                   || node.isTokenTree(lexer::TokenType::OpenParen)
                   || node.isTopLevelStarter();
        });

        if (!decl.genericConstraints.empty()) {
            decl.endPos = decl.genericConstraints.back().end();
        }

        if (it.isEnd() || it->isTopLevelStarter()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingStructBody, decl.startPos);
            error.addLabel("expected struct body", *(it - 1));
            addError(std::move(error));
            modules.back().structDeclarations.emplace_back(std::move(decl));
            return;
        }

        if (const auto tree = tryConsumeTokenTree(it, lexer::TokenType::OpenParen)) {
            auto parameters = parameterListRule(**tree);
            for (auto &parameter: parameters) {
                if (parameter.isMut || parameter.isRef) {
                    auto error = errors::CompilerError(errors::ErrorCode::InvalidModifier, parameter.startPos);
                    error.setNote("short body struct properties cannot be `mut` and are `pub` by default");
                    addError(std::move(error));
                }

                auto prop = ast::PropertyDeclaration(parameter.name);
                prop.type = std::move(parameter.type);
                prop.isPublic = true;
                prop.startPos = parameter.start();
                prop.endPos = parameter.end();

                decl.propertyDeclarations.push_back(std::move(prop));

                decl.destructureProperties.push_back(parameter.name);
            }
            decl.endPos = (*tree)->right.getEnd();

            beforeRecover = it;
            recoverUntil(it, [](const auto &node) {
                return node.isTopLevelStarter()
                       || node.isToken(lexer::TokenType::Semicolon)
                       || node.isToken(lexer::TokenType::Where);
            });
            if (beforeRecover != it) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
            }
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
                modules.back().structDeclarations.emplace_back(std::move(decl));
                return;
            }

            decl.genericConstraints = genericConstraintListRule(it, [](const auto &node) {
                return node.isToken(lexer::TokenType::Semicolon)
                       || node.isTopLevelStarter();
            });

            if (it.isEnd() || !it->isToken(lexer::TokenType::Semicolon)) {
                auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd());
                addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
            }
            it += 1;
        } else if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
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
            recoverUntil(it, [](const auto &node) {
                return node.isTopLevelStarter()
                       || node.isToken(lexer::TokenType::DestructuresInto);
            });
            if (it && it->isToken(lexer::TokenType::DestructuresInto)) {
                decl.endPos = it->getEnd();
                it += 1;
                if (it.isEnd()) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, (it - 1)->getEnd()));
                    modules.back().structDeclarations.emplace_back(std::move(decl));
                    return;
                }
                decl.destructureProperties = identifierListRule(consumeTokenTree(it, lexer::TokenType::OpenParen));


                beforeRecover = it;
                recoverUntil(it, [](const auto &node) {
                    return node.isTopLevelStarter()
                           || node.isToken(lexer::TokenType::Semicolon);
                });

                if (beforeRecover != it) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
                }
                if (it.isEnd() || !it->isToken(lexer::TokenType::Semicolon)) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
                }
                it += 1;
            }
        } else {
            COMPILER_UNREACHABLE();
        }

        modules.back().structDeclarations.emplace_back(std::move(decl));
    }

    std::vector<ast::ConstraintDeclaration> Parser::genericConstraintListRule(TokenTreeIterator &it,
                                                                              const RecoverPredicate &recoverPredicate) {
        std::vector<ast::ConstraintDeclaration> result;
        while (it && it->isToken(lexer::TokenType::Where)) {
            auto constraint = genericConstraintRule(it);
            if (constraint) {
                result.emplace_back(std::move(*constraint));
            }
            auto beforeRecover = it;
            recoverUntil(it, [&recoverPredicate](const auto &node) {
                return node.isToken(lexer::TokenType::Where) || recoverPredicate(node);
            });
            if (beforeRecover != it) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
            }
        }
        return result;
    }


    std::optional<ast::PropertyDeclaration> Parser::propertyDeclarationRule(TokenTreeIterator &it) {
        auto hasRecovered = recoverUntil(it, [](const auto &node) {
            return node.isModifier() || node.isToken(lexer::TokenType::Identifier) || node.isToken(lexer::TokenType::Semicolon);
        });
        if (it.isEnd()) {
            //BEAN
            return std::nullopt;
        }
        if (auto tok = tryConsumeToken(it, lexer::TokenType::Semicolon)) {
            if (!hasRecovered) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, tok->start));
            }
            return std::nullopt;
        }

        auto modifiers = modifierRule(it, [](const auto &) { return true; });
        validateModifiers(modifiers, {lexer::TokenType::Pub, lexer::TokenType::Mut});

        hasRecovered = recoverUntil(it, {lexer::TokenType::Identifier, lexer::TokenType::Semicolon});

        if (it.isEnd()) {
            return std::nullopt;
        }
        if (auto tok = tryConsumeToken(it, lexer::TokenType::Semicolon)) {
            if (!hasRecovered) {
                addError(errors::CompilerError(errors::ErrorCode::MissingPropertyName, tok->start));
            }
            return std::nullopt;
        }

        if (!it->isToken(lexer::TokenType::Identifier)) {
            addError(errors::CompilerError(errors::ErrorCode::MissingPropertyName, it->getStart()));
            return std::nullopt;
        }

        ast::PropertyDeclaration prop(identifierRule(it));
        if (!modifiers.empty()) {
            prop.startPos = modifiers.front().start;
        } else {
            prop.startPos = prop.name.start();
        }
        prop.isMutable = containsModifier(modifiers, lexer::TokenType::Mut);
        prop.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);

        prop.endPos = prop.name.start();
        recoverUntil(it, [&](const auto &node) {
            return node.isSignatureStarter() || node.isToken(lexer::TokenType::Colon) || node.isToken(lexer::TokenType::Semicolon);
        });

        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingPropertyType, it->getStart()));
            return prop;
        }
        if (auto tok = tryConsumeToken(it, lexer::TokenType::Colon)) {
            prop.endPos = tok->end;
            recoverUntil(it, [&](const auto &node) {
                return node.isSignatureStarter() || node.isToken(lexer::TokenType::Semicolon);
            });

            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingPropertyType, it->getStart()));
                return prop;
            }
        }
        if (!it->isSignatureStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingPropertyType, it->getStart()));
            return prop;
        }

        prop.type = signatureRule(it);
        prop.endPos = prop.type->end();

        recoverUntil(it, [&](const auto &node) {
            return node.isModifier() || node.isToken(lexer::TokenType::Identifier) || node.isToken(lexer::TokenType::Semicolon);
        });

        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
            return prop;
        }

        if (auto tok = tryConsumeToken(it, lexer::TokenType::Semicolon)) {
            prop.endPos = tok->end;
        } else {
            addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd()));
        }

        return prop;
    }

    void Parser::functionRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        const auto fnToken = consumeToken(it, lexer::TokenType::Fn);
        auto &decl = modules.back().functionDeclarations.emplace_back();
        if (!modifiers.empty()) {
            decl.startPos = modifiers.front().start;
        } else {
            decl.startPos = fnToken.start;
        }
        decl.endPos = fnToken.end;

        validateModifiers(modifiers, {lexer::TokenType::Pub});
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(lexer::TokenType::Identifier)
                   || n.isTokenTree(lexer::TokenType::OpenAngle)
                   || n.isTokenTree(lexer::TokenType::OpenParen)
                   || n.isToken(lexer::TokenType::DashArrow)
                   || n.isToken(lexer::TokenType::Where)
                   || n.isToken(lexer::TokenType::EqualArrow)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingDeclarationName, decl.start()));
            return;
        }

        if (it->isToken(lexer::TokenType::Identifier)) {
            decl.name = identifierRule(it);
            decl.endPos = decl.name->end();

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTokenTree(lexer::TokenType::OpenAngle)
                       || n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::MissingDeclarationName, decl.start()));
        }

        if (it->isTokenTree(lexer::TokenType::OpenAngle)) {
            decl.genericParams = identifierListRule(consumeTokenTree(it, lexer::TokenType::OpenAngle));

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return;
            }
        }

        if (it->isTokenTree(lexer::TokenType::OpenParen)) {
            decl.parameters = parameterListRule(consumeTokenTree(it, lexer::TokenType::OpenParen));

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::MissingMethodParams, decl.start()));
        }

        decl.returnType = returnTypeRule(it);
        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isToken(lexer::TokenType::Where)
                   || n.isToken(lexer::TokenType::EqualArrow)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return;
        }

        if (it->isToken(lexer::TokenType::Where)) {
            decl.genericConstraints = genericConstraintListRule(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(lexer::TokenType::Where)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return;
            }
        }

        if (it->isToken(lexer::TokenType::EqualArrow)) {
            consumeToken(it, lexer::TokenType::EqualArrow);
            if (it->isToken(lexer::TokenType::Semicolon)) {
                consumeToken(it, lexer::TokenType::Semicolon);
            } else {
                addError(errors::CompilerError(errors::ErrorCode::MissingSemicolon, decl.start()));
            }
        } else if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
            consumeTokenTree(it, lexer::TokenType::OpenCurly);
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return;
        }
    }

    void Parser::aliasRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        COMPILER_ASSERT(it->isToken(lexer::TokenType::Alias),
                        "aliasRule called with non-alias starting token");

        auto decl = ast::AliasDeclaration();
        if (!modifiers.empty()) {
            decl.startPos = modifiers.begin()->start;
        } else {
            decl.startPos = it->getStart();
        }
        decl.endPos = it->getEnd();
        it += 1;

        validateModifiers(modifiers, {lexer::TokenType::Pub});
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);

        auto beforeRecover = it;
        recoverUntil(it, [](const auto &node) {
            return node.isTopLevelStarter() || node.isToken(lexer::TokenType::Identifier);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingDeclarationName, (it - 1)->getEnd()));
            return;
        }

        if (beforeRecover != it) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }

        decl.name = identifierRule(it);
        if (!decl.name) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingDeclarationName, decl.startPos);
            error.addLabel("expected alias name", *it);
            addError(std::move(error));
            return;
        }
        decl.endPos = decl.name->end();

        beforeRecover = it;
        recoverUntil(it, [](const auto &node) {
            return node.isTopLevelStarter() || node.isSignatureStarter() || node.isToken(lexer::TokenType::Equals) || node.
                    isTokenTree(lexer::TokenType::OpenAngle);
        });

        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::MissingAliasType, decl.start()));
            modules.back().aliasDeclarations.emplace_back(std::move(decl));
            return;
        }
        if (it != beforeRecover) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
        if (const auto tree = tryConsumeTokenTree(it, lexer::TokenType::OpenAngle)) {
            decl.genericParams = identifierListRule(**tree);

            beforeRecover = it;
            recoverUntil(it, [](const auto &node) {
                return node.isTopLevelStarter() || node.isSignatureStarter() || node.isToken(lexer::TokenType::Equals);
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingAliasType, decl.start()));
                modules.back().aliasDeclarations.emplace_back(std::move(decl));
                return;
            }
            if (it != beforeRecover) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
            }
        }
        if (it->isToken(lexer::TokenType::Equals)) {
            it += 1;
            beforeRecover = it;
            recoverUntil(it, [](const auto &node) {
                return node.isTopLevelStarter() || node.isSignatureStarter();
            });

            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::MissingAliasType, decl.start()));
                modules.back().aliasDeclarations.emplace_back(std::move(decl));
                return;
            }
            if (it != beforeRecover) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::MissingEquals, decl.start()));
        }


        auto signature = signatureRule(it);
        decl.signature = std::move(signature);
        decl.endPos = decl.signature->end();


        decl.genericConstraints = genericConstraintListRule(it, [](const auto &node) {
            return node.isToken(lexer::TokenType::Semicolon)
                   || node.isTopLevelStarter();
        });
        if (!decl.genericConstraints.empty()) {
            decl.endPos = decl.genericConstraints.back().end();
        }

        if (it.isEnd() || it->isTopLevelStarter()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, (it - 1)->getEnd());
        } else {
            COMPILER_ASSERT(it->isToken(lexer::TokenType::Semicolon), "unexpected lexer::Token type");
            it += 1;
        }

        modules.back().aliasDeclarations.emplace_back(std::move(decl));
    }

    void Parser::moduleVariableRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        COMPILER_ASSERT(it->isToken(lexer::TokenType::Let),
                        "moduleVariableRule called with non-let starting token");

        auto decl = ast::ModuleVariableDeclaration();
        if (!modifiers.empty()) {
            decl.startPos = modifiers.begin()->start;
        } else {
            decl.startPos = it->getStart();
        }
        decl.endPos = it->getEnd();
        it += 1;

        validateModifiers(modifiers, {lexer::TokenType::Pub, lexer::TokenType::Mut});
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);
        decl.isMut = containsModifier(modifiers, lexer::TokenType::Mut);

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingDeclarationName, decl.startPos);
            error.addLabel("expected variable name", *(it - 1));
            addError(std::move(error));
            modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
            return;
        }

        decl.name = identifierRule(it);
        if (!decl.name) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingDeclarationName, decl.startPos);
            error.addLabel("expected variable name", *it);
            addError(std::move(error));
        }
        decl.endPos = decl.name->end();

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, decl.startPos);
            error.addLabel("missing semicolon", *(it - 1));
            addError(std::move(error));
            modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
            return;
        }

        if (!it->isToken(lexer::TokenType::Colon)) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.startPos);
            error.addLabel("expected colon: `:`", *it);
            addError(std::move(error));
            modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
            return;
        }

        decl.endPos = it->getEnd();
        it += 1;

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingVariableType, decl.startPos);
            error.addLabel("expected a type signature", *(it - 1));
            addError(std::move(error));
            modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
            return;
        }

        decl.type = signatureRule(it);

        if (!decl.type) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingVariableType, decl.startPos);
            error.addLabel("expected a type signature", *it);
            addError(std::move(error));
            modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
            return;
        }

        if (it.isEnd()) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, (it - 1)->getEnd());
            error.setNote("unexpected end of method declaration, expected `;`");
            addError(std::move(error));
            modules.back().moduleVariableDeclarations.emplace_back(std::move(decl));
            return;
        }

        if (!it->isToken(lexer::TokenType::Semicolon)) {
            auto error = errors::CompilerError(errors::ErrorCode::MissingSemicolon, decl.start());
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
                auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, it->getStart());
                error.addLabel("expected a top level declaration", *it);
                addError(std::move(error));
                recoverTopLevel(it);
                return;
            }
            auto &token = tokenResult.get();

            if (token.type == lexer::TokenType::Enum) {
                enumRule(it, std::move(modifiers));
            } else if (token.type == lexer::TokenType::Interface) {
                interfaceRule(it, std::move(modifiers));
            } else if (token.type == lexer::TokenType::Struct) {
                structRule(it, std::move(modifiers));
            } else if (token.type == lexer::TokenType::Fn) {
                functionRule(it, std::move(modifiers));
            } else if (token.type == lexer::TokenType::Alias) {
                aliasRule(it, std::move(modifiers));
            } else if (token.type == lexer::TokenType::Let) {
                moduleVariableRule(it, std::move(modifiers));
            } else {
                auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, it->getStart());
                error.addLabel("expected a top level declaration", *it);
                addError(std::move(error));
                recoverTopLevel(it);
            }
        } else {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, it->getStart());
            error.addLabel("expected a top level declaration", *it);
            addError(std::move(error));
            recoverTopLevel(it);
        }
    }

    void Parser::implRule(TokenTreeIterator &it) {
        const auto implToken = consumeToken(it, lexer::TokenType::Impl);
        auto &impl = modules.back().implBlocks.emplace_back();
        impl.startPos = implToken.start;

        recoverUntil(it, [](const auto &n) {
            return n.isTopLevelStarter()
                   || n.isTokenTree(lexer::TokenType::OpenAngle)
                   || n.isTypeSignatureStarter()
                   || n.isToken(lexer::TokenType::For)
                   || n.isToken(lexer::TokenType::Where)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd() || it->isTopLevelStarter()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
            return;
        }

        // generics
        if (it->isTokenTree(lexer::TokenType::OpenAngle)) {
            impl.genericParams = identifierListRule(consumeTokenTree(it, lexer::TokenType::OpenAngle));

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTypeSignatureStarter()
                       || n.isToken(lexer::TokenType::For)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                return;
            }
        }

        // (interface on?) typename
        if (it->isTypeSignatureStarter()) {
            auto signature = typeSignatureRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isToken(lexer::TokenType::On)
                       || n.isToken(lexer::TokenType::For)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                return;
            }

            if (tryConsumeToken(it, lexer::TokenType::On)) {
                impl.interfaceName = std::move(signature);
                impl.structName = typeSignatureRule(it);
                recoverUntil(it, [](const auto &n) {
                    return n.isTopLevelStarter()
                           || n.isToken(lexer::TokenType::For)
                           || n.isToken(lexer::TokenType::Where)
                           || n.isTokenTree(lexer::TokenType::OpenCurly);
                });
                if (it.isEnd() || it->isTopLevelStarter()) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                    return;
                }
            } else {
                impl.structName = std::move(signature);
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, impl.start()));
            return;
        }

        // for interface?
        if (tryConsumeToken(it, lexer::TokenType::For)) {
            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTypeSignatureStarter()
                       || n.isToken(lexer::TokenType::Where)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                return;
            }

            if (it->isTypeSignatureStarter()) {
                impl.forInterfaceName = typeSignatureRule(it);

                recoverUntil(it, [](const auto &n) {
                    return n.isTopLevelStarter()
                           || n.isToken(lexer::TokenType::Where)
                           || n.isTokenTree(lexer::TokenType::OpenCurly);
                });
                if (it.isEnd() || it->isTopLevelStarter()) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                    return;
                }
            } else {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, impl.start()));
                return;
            }
        }

        // generic constraints
        if (it->isToken(lexer::TokenType::Where)) {
            impl.genericConstraints = genericConstraintListRule(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            recoverUntil(it, [](const auto &n) {
                return n.isTopLevelStarter()
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                return;
            }
        }

        //  body
        if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
            const auto &body = consumeTokenTree(it, lexer::TokenType::OpenCurly);

            auto bodyIt = TokenTreeIterator(body.tokens);
            while (!bodyIt.isEnd()) {
                recoverUntil(bodyIt, [](auto const &n) {
                    return n.isModifier()
                           || n.isToken(lexer::TokenType::Identifier)
                           || n.isToken(lexer::TokenType::Tilde)
                           || n.isToken(lexer::TokenType::Fn)
                           || n.isToken(lexer::TokenType::Get)
                           || n.isToken(lexer::TokenType::Set);
                });

                if (bodyIt.isEnd()) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                    break;
                }

                std::vector<lexer::Token> modifiers;
                if (bodyIt->isModifier()) {
                    modifiers = modifierRule(bodyIt, [](auto const &n) {
                        return n.isToken(lexer::TokenType::Identifier)
                               || n.isToken(lexer::TokenType::Tilde)
                               || n.isToken(lexer::TokenType::Fn)
                               || n.isToken(lexer::TokenType::Get)
                               || n.isToken(lexer::TokenType::Set);
                    });
                }

                if (bodyIt.isEnd()) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, impl.start()));
                    break;
                }

                if (bodyIt->isToken(lexer::TokenType::Identifier)) {
                    impl.constructors.emplace_back(constructorRule(bodyIt, modifiers));
                } else if (bodyIt->isToken(lexer::TokenType::Tilde)) {
                    impl.destructors.emplace_back(destructorRule(bodyIt, modifiers));
                } else if (bodyIt->isToken(lexer::TokenType::Fn)) {
                    impl.methods.emplace_back(methodRule(bodyIt, modifiers));
                } else if (bodyIt->isToken(lexer::TokenType::Get)) {
                    impl.getters.emplace_back(implGetterRule(bodyIt, modifiers));
                } else if (bodyIt->isToken(lexer::TokenType::Set)) {
                    impl.setters.emplace_back(implSetterRule(bodyIt, modifiers));
                }
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, impl.start()));
        }
    }

    ast::ConstructorDeclaration Parser::constructorRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        validateModifiers(modifiers, {lexer::TokenType::Pub});
        ast::ConstructorDeclaration decl;
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);

        decl.name = identifierRule(it);

        recoverUntil(it, [](const auto &n) {
            return n.isTokenTree(lexer::TokenType::OpenParen)
                   || n.isToken(lexer::TokenType::Colon)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return decl;
        }

        if (it->isTokenTree(lexer::TokenType::OpenParen)) {
            decl.parameters = parameterListRule(consumeTokenTree(it, lexer::TokenType::OpenParen));

            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::Colon)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd() || it->isTopLevelStarter()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        if (tryConsumeToken(it, lexer::TokenType::Colon)) {
            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::Identifier)
                       || n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }

            if (it->isToken(lexer::TokenType::Identifier)) {
                decl.otherName = identifierRule(it);

                recoverUntil(it, [](const auto &n) {
                    return n.isTokenTree(lexer::TokenType::OpenParen)
                           || n.isTokenTree(lexer::TokenType::OpenCurly);
                });
                if (it.isEnd()) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                    return decl;
                }
            } else {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            }

            if (it->isTokenTree(lexer::TokenType::OpenParen)) {
                consumeTokenTree(it, lexer::TokenType::OpenParen);
                //TODO: expressions

                recoverUntil(it, [](const auto &n) {
                    return n.isTokenTree(lexer::TokenType::OpenCurly);
                });
                if (it.isEnd()) {
                    addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                    return decl;
                }
            } else {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            }

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        }

        if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
            consumeTokenTree(it, lexer::TokenType::OpenCurly);
            //TODO: expressions
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
        }

        return decl;
    }

    ast::DestructureDeclaration Parser::destructorRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        validateModifiers(modifiers, {});
        ast::DestructureDeclaration decl;

        consumeToken(it, lexer::TokenType::Tilde);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(lexer::TokenType::Identifier)
                   || n.isTokenTree(lexer::TokenType::OpenParen)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return decl;
        }

        if (it->isToken(lexer::TokenType::Identifier)) {
            decl.name = identifierRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        if (tryConsumeTokenTree(it, lexer::TokenType::OpenParen)) {
            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
            consumeTokenTree(it, lexer::TokenType::OpenCurly);
            //TODO: statements!
        }

        return decl;
    }

    ast::ImplMethod Parser::methodRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        validateModifiers(modifiers, {lexer::TokenType::Pub, lexer::TokenType::Mut, lexer::TokenType::Static});
        ast::ImplMethod decl;
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);
        decl.isMut = containsModifier(modifiers, lexer::TokenType::Mut);
        decl.isStatic = containsModifier(modifiers, lexer::TokenType::Static);

        consumeToken(it, lexer::TokenType::Fn);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(lexer::TokenType::Identifier)
                   || n.isTokenTree(lexer::TokenType::OpenAngle)
                   || n.isTokenTree(lexer::TokenType::OpenParen)
                   || n.isToken(lexer::TokenType::DashArrow)
                   || n.isToken(lexer::TokenType::Where)
                   || n.isToken(lexer::TokenType::EqualArrow)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return decl;
        }

        if (it->isToken(lexer::TokenType::Identifier)) {
            decl.name = identifierRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(lexer::TokenType::OpenAngle)
                       || n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        if (it->isTokenTree(lexer::TokenType::OpenAngle)) {
            decl.genericParams = identifierListRule(consumeTokenTree(it, lexer::TokenType::OpenAngle));

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        }

        if (it->isTokenTree(lexer::TokenType::OpenParen)) {
            decl.parameters = parameterListRule(consumeTokenTree(it, lexer::TokenType::OpenParen));

            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::Where)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        decl.returnType = returnTypeRule(it);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(lexer::TokenType::Where)
                   || n.isToken(lexer::TokenType::EqualArrow)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return decl;
        }

        if (it->isToken(lexer::TokenType::Where)) {
            decl.genericConstraints = genericConstraintListRule(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });

            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        }

        if (it->isToken(lexer::TokenType::EqualArrow)) {
            consumeToken(it, lexer::TokenType::EqualArrow);
            //TODO: expressions
            consumeToken(it, lexer::TokenType::Semicolon);
        } else if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
            consumeTokenTree(it, lexer::TokenType::OpenCurly);
            //TODO: statements
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        return decl;
    }

    ast::ImplGetter Parser::implGetterRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        validateModifiers(modifiers, {lexer::TokenType::Pub, lexer::TokenType::Mut});
        ast::ImplGetter decl;
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);
        decl.isMut = containsModifier(modifiers, lexer::TokenType::Mut);

        consumeToken(it, lexer::TokenType::Get);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(lexer::TokenType::Identifier)
                   || n.isTokenTree(lexer::TokenType::OpenParen)
                   || n.isToken(lexer::TokenType::DashArrow)
                   || n.isToken(lexer::TokenType::EqualArrow)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return decl;
        }

        if (it->isToken(lexer::TokenType::Identifier)) {
            decl.name = identifierRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        if (it->isTokenTree(lexer::TokenType::OpenParen)) {
            consumeTokenTree(it, lexer::TokenType::OpenParen);

            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::DashArrow)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        decl.returnType = returnTypeRule(it);
        if (decl.returnType) {
            decl.endPos = decl.returnType->end();
        } else {
            addError(errors::CompilerError(errors::ErrorCode::MissingReturnType, (it - 1)->getEnd()));
        }

        recoverUntil(it, [](const auto &n) {
            return n.isToken(lexer::TokenType::EqualArrow)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, (it - 1)->getEnd()));
            return decl;
        }

        if (it->isToken(lexer::TokenType::EqualArrow)) {
            consumeToken(it, lexer::TokenType::EqualArrow);
            consumeToken(it, lexer::TokenType::Semicolon);
            //TODO: expressions
        } else if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
            consumeTokenTree(it, lexer::TokenType::OpenCurly);
            //TODO: statements
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        return decl;
    }

    ast::ImplSetter Parser::implSetterRule(TokenTreeIterator &it, std::vector<lexer::Token> modifiers) {
        validateModifiers(modifiers, {lexer::TokenType::Pub});
        ast::ImplSetter decl;
        decl.isPublic = containsModifier(modifiers, lexer::TokenType::Pub);

        consumeToken(it, lexer::TokenType::Set);

        recoverUntil(it, [](const auto &n) {
            return n.isToken(lexer::TokenType::Identifier)
                   || n.isTokenTree(lexer::TokenType::OpenParen)
                   || n.isToken(lexer::TokenType::EqualArrow)
                   || n.isTokenTree(lexer::TokenType::OpenCurly);
        });
        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
            return decl;
        }

        if (it->isToken(lexer::TokenType::Identifier)) {
            decl.name = identifierRule(it);

            recoverUntil(it, [](const auto &n) {
                return n.isTokenTree(lexer::TokenType::OpenParen)
                       || n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        if (it->isTokenTree(lexer::TokenType::OpenParen)) {
            auto params = parameterListRule(consumeTokenTree(it, lexer::TokenType::OpenParen));
            if (params.size() != 1) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
            }
            decl.parameter = std::move(params[0]);

            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::EqualArrow)
                       || n.isTokenTree(lexer::TokenType::OpenCurly);
            });
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::UnexpectedEndOfInput, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        if (it->isToken(lexer::TokenType::EqualArrow)) {
            consumeToken(it, lexer::TokenType::EqualArrow);
            consumeToken(it, lexer::TokenType::Semicolon);
            //TODO: expressions
        } else if (it->isTokenTree(lexer::TokenType::OpenCurly)) {
            consumeTokenTree(it, lexer::TokenType::OpenCurly);
            //TODO: statements
        } else {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.start()));
        }

        return decl;
    }

    ast::Path Parser::pathRule(TokenTreeIterator &it, const bool allowTrailing) {
        ast::Path path{};

        if ((path.rooted = tryConsumeToken(it, lexer::TokenType::PathSeparator))) {
            recoverUntil(it, lexer::TokenType::Identifier);
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::EmptyPath, path.rooted->start));
                return path;
            }
        }

        path.parts.push_back(identifierRule(it));

        while (it && it->isToken(lexer::TokenType::PathSeparator)) {
            const auto fallback = it;
            auto trailer = consumeToken(it, lexer::TokenType::PathSeparator);
            if (it.isEnd() || !it->isToken(lexer::TokenType::Identifier)) {
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

    std::optional<ast::ConstraintDeclaration> Parser::genericConstraintRule(TokenTreeIterator &it) {
        COMPILER_ASSERT(it && it->isToken(lexer::TokenType::Where),
                        "constraint rule called but start lexer::Token is not a `where`");
        it += 1;
        auto decl = ast::ConstraintDeclaration();

        // TODO: error recovery?

        decl.name = identifierRule(it);
        if (!decl.name) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.startPos);
            error.addLabel("expected generic parameter name", *it);
            addError(std::move(error));
            return std::nullopt;
        }
        decl.startPos = decl.name->start();
        decl.endPos = decl.name->end();

        // TODO: error recovery and end checking
        if (!it->isToken(lexer::TokenType::Colon)) {
            auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, decl.startPos);
            error.addLabel("expected `:`", *it);
            addError(std::move(error));
        } else {
            it += 1;
        }

        while (it) {
            if (auto c1 = interfaceConstraintRule(it)) {
                decl.constraints.emplace_back(std::make_unique<ast::InterfaceConstraint>(std::move(*c1)));
            } /* else if (auto c2 = defaultConstraintRule(it)) {
            decl.constraints.push_back(std::move(c2));
        }*/
            else if (it->isConstraintBreakout()) {
                break;
            } else {
                auto error = errors::CompilerError(errors::ErrorCode::InvalidGenericConstraint, decl.startPos);
                error.addLabel("expected generic constraint", *it);
                addError(std::move(error));
                recoverUntil(it, [](const auto &node) {
                    return node.isConstraintBreakout() || node.isToken(lexer::TokenType::Comma);
                });
            }

            if (it && it->isToken(lexer::TokenType::Comma)) {
                it += 1;
            } else {
                break;
            }
        }


        decl.endPos = it->getEnd();
        return decl;
    }

    std::optional<ast::InterfaceConstraint> Parser::interfaceConstraintRule(TokenTreeIterator &it) {
        ast::InterfaceConstraint constraint;
        constraint.typeSignature = typeSignatureRule(it);
        return constraint;
    }

    ast::Identifier Parser::identifierRule(TokenTreeIterator &it) const {
        return ast::Identifier::make(consumeToken(it, lexer::TokenType::Identifier), source);
    }

    ast::Signature Parser::signatureRule(TokenTreeIterator &it) {
        COMPILER_ASSERT(it && it->isSignatureStarter(), "signatureRule called with non signature starter");

        if (it->isPathStarter()) {
            return typeSignatureRule(it);
        }
        if (it->isToken(lexer::TokenType::Fn)) {
            return functionSignatureRule(it);
        }
        if (it->isTokenTree(lexer::TokenType::OpenParen)) {
            return tupleSignatureRule(it);
        }

        COMPILER_ASSERT(false, "signature rule called with wrong input");
    }

    ast::TypeSignature Parser::typeSignatureRule(TokenTreeIterator &it) {
        auto decl = ast::TypeSignature();

        decl.path = pathRule(it, false);
        decl.startPos = decl.path.start();
        decl.endPos = decl.path.end();

        if (it.isEnd()) {
            return decl;
        }

        if (const auto tree = tryConsumeTokenTree(it, lexer::TokenType::OpenAngle)) {
            decl.genericStartPos = (*tree)->left.start;
            decl.genericEndPos = (*tree)->right.getEnd();
            decl.genericArguments = signatureListRule(**tree);
            decl.endPos = (*tree)->right.getEnd();
        }

        return decl;
    }

    ast::FunctionSignature Parser::functionSignatureRule(TokenTreeIterator &it) {
        const auto fnToken = consumeToken(it, lexer::TokenType::Fn);
        auto decl = ast::FunctionSignature();
        decl.startPos = fnToken.start;
        decl.endPos = fnToken.end;

        if (it.isEnd()) {
            addError(errors::CompilerError(errors::ErrorCode::FnSignatureMissingParams, decl.start()));
            return decl;
        }

        if (const auto parens = tryConsumeTokenTree(it, lexer::TokenType::OpenParen)) {
            decl.parameterTypes = namelessParameterListRule(**parens);
            decl.endPos = (*parens)->right.getEnd();

            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::FnSignatureMissingParams, decl.start()));
                return decl;
            }
        } else {
            addError(errors::CompilerError(errors::ErrorCode::MissingReturnType, it->getStart()));
        }

        if (auto returnType = returnTypeRule(it)) {
            decl.returnType = std::make_unique<ast::ReturnType>(std::move(*returnType));
            decl.endPos = decl.returnType->end();
        }

        return decl;
    }

    ast::TupleSignature Parser::tupleSignatureRule(TokenTreeIterator &it) {
        auto decl = ast::TupleSignature();

        auto const &list = consumeTokenTree(it, lexer::TokenType::OpenParen);
        decl.startPos = list.left.start;
        decl.endPos = list.right.getEnd();

        decl.types = signatureListRule(list);

        return decl;
    }

    std::optional<ast::ReturnType> Parser::returnTypeRule(TokenTreeIterator &it) {
        auto dashArrowToken = tryConsumeToken(it, lexer::TokenType::DashArrow);
        if (!dashArrowToken) {
            return std::nullopt;
        }

        auto modifiers = modifierRule(it, [](const auto &n) {
            return true;
        });
        validateModifiers(modifiers, {lexer::TokenType::Mut});

        if (it.isEnd() || !(it->isSignatureStarter())) {
            addError(errors::CompilerError(errors::ErrorCode::MissingReturnType, (it - 1)->getEnd()));
            return std::nullopt;
        }

        ast::ReturnType result(signatureRule(it));
        result.isMut = containsModifier(modifiers, lexer::TokenType::Mut);
        result.startPos = dashArrowToken->start;
        result.endPos = result.type.end();

        return result;
    }

    std::vector<ast::Identifier> Parser::identifierListRule(const lexer::TokenTree &list) {
        std::vector<ast::Identifier> result;

        auto it = TokenTreeIterator(list.tokens);

        while (it) {
            recoverUntil(it, lexer::TokenType::Identifier);
            if (it.isEnd()) {
                break;
            }
            result.emplace_back(identifierRule(it));

            if (it.isEnd()) {
                break;
            }
            if (!tryConsumeToken(it, lexer::TokenType::Comma)) {
                addError(errors::CompilerError(errors::ErrorCode::MissingComma, (it - 1)->getEnd()));
            }
        }

        return result;
    }

    std::vector<ast::Parameter> Parser::parameterListRule(const lexer::TokenTree &list) {
        std::vector<ast::Parameter> result;

        auto it = TokenTreeIterator(list.tokens);
        while (it) {
            auto startPos = it->getStart();
            auto modifiers = modifierRule(it, [](const auto &) { return true; });
            validateModifiers(modifiers, {lexer::TokenType::Mut, lexer::TokenType::Ref});

            auto beforeRecover = it;
            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::Identifier) || n.isToken(lexer::TokenType::Comma) || n.isToken(lexer::TokenType::Colon);
            });
            if (beforeRecover != it) {
                auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart());
                error.addLabel("unexpected token", *beforeRecover);
                addError(std::move(error));
            }
            if (it.isEnd() || tryConsumeToken(it, lexer::TokenType::Comma)) {
                addError(errors::CompilerError(errors::ErrorCode::ParameterNameMissing, (it - 1)->getEnd()));
                continue;
            }
            if (tryConsumeToken(it, lexer::TokenType::Colon)) {
                recoverUntil(it, lexer::TokenType::Comma);
                continue;
            }

            auto identifier = identifierRule(it);

            auto &param = result.emplace_back(identifier);
            param.startPos = startPos;
            param.endPos = identifier.end();
            param.isMut = containsModifier(modifiers, lexer::TokenType::Mut);
            param.isRef = containsModifier(modifiers, lexer::TokenType::Ref);
            if (param.isMut && param.isRef) {
                auto error = errors::CompilerError(errors::ErrorCode::RefAlreadyImpliesMut, modifiers.front());
                error.setNote("redundant `mut` specification, `ref` implies `mut`");
                addError(std::move(error));
            }

            beforeRecover = it;
            recoverUntil(it, [](const auto &n) {
                return n.isSignatureStarter() || n.isToken(lexer::TokenType::Colon) || n.isToken(lexer::TokenType::Comma);
            });
            if (beforeRecover != it) {
                auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart());
                error.addLabel("unexpected token", *beforeRecover);
                addError(std::move(error));
            }
            if (it.isEnd()) {
                addError(errors::CompilerError(errors::ErrorCode::ParameterTypeMissing, (it - 1)->getEnd()));
                continue;
            }
            if (it->isToken(lexer::TokenType::Comma)) {
                addError(errors::CompilerError(errors::ErrorCode::ParameterTypeMissing, it->getStart()));
                param.endPos = it->getEnd();
                it += 1;
                continue;
            }

            if (!it->isToken(lexer::TokenType::Colon)) {
                auto error = errors::CompilerError(errors::ErrorCode::MissingColon, it->getStart());
                error.addLabel("expected `:` to separate the parameter name and type", *it);
                addError(std::move(error));
            } else {
                param.endPos = it->getEnd();
                it += 1;
                beforeRecover = it;
                recoverUntil(it, [](const auto &node) {
                    return node.isSignatureStarter() || node.isToken(lexer::TokenType::Comma);
                });
                if (beforeRecover != it) {
                    auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart());
                    error.addLabel("unexpected token", *beforeRecover);
                    addError(std::move(error));
                }
                if (it.isEnd() || it->isToken(lexer::TokenType::Comma)) {
                    addError(errors::CompilerError(errors::ErrorCode::ParameterTypeMissing, (it - 1)->getEnd()));
                    continue;
                }
            }
            COMPILER_ASSERT(it && it->isSignatureStarter(),
                            "it must be a signature starter at this point");


            param.type = signatureRule(it);
            if (param.type) {
                param.endPos = param.type->end();
            }
            beforeRecover = it;
            recoverUntil(it, lexer::TokenType::Comma); // TODO: move unexpected lexer::Token error into recoverUntil?
            if (it != beforeRecover) {
                auto error = errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart());
                error.addLabel("unexpected token", *beforeRecover);
                addError(std::move(error));
            }

            DEBUG_ASSERT(it.isEnd() || it->isToken(lexer::TokenType::Comma), "expeced end or comma");

            it += 1;
        }

        return result;
    }

    std::vector<ast::NamelessParameter> Parser::namelessParameterListRule(const lexer::TokenTree &list) {
        std::vector<ast::NamelessParameter> result;
        auto it = TokenTreeIterator(list.tokens);
        while (it) {
            auto modifiers = modifierRule(it, [](const auto &) { return true; });
            validateModifiers(modifiers, {lexer::TokenType::Mut, lexer::TokenType::Ref});

            recoverUntil(it, [](const auto &n) {
                return n.isSignatureStarter() || n.isToken(lexer::TokenType::Comma);
            });

            if (it.isEnd() || tryConsumeToken(it, lexer::TokenType::Comma)) {
                addError(errors::CompilerError(errors::ErrorCode::ParameterTypeMissing, (it - 1)->getEnd()));
                continue;
            }

            auto &param = result.emplace_back(signatureRule(it));
            if (modifiers.empty()) {
                param.startPos = param.type.start();
            } else {
                param.startPos = modifiers.front().start;
            }
            param.endPos = param.type.end();
            param.isMut = containsModifier(modifiers, lexer::TokenType::Mut);
            param.isRef = containsModifier(modifiers, lexer::TokenType::Ref);
            if (param.isMut && param.isRef) {
                auto error = errors::CompilerError(errors::ErrorCode::RefAlreadyImpliesMut, modifiers.front());
                error.setNote("redundant `mut` specification, `ref` implies `mut`");
                addError(std::move(error));
            }

            recoverUntil(it, [](const auto &n) {
                return n.isToken(lexer::TokenType::Comma) || n.isSignatureStarter() || n.isModifier();
            });

            if (it.isEnd()) {
                break;
            }

            if (!tryConsumeToken(it, lexer::TokenType::Comma)) {
                addError(errors::CompilerError(errors::ErrorCode::MissingComma, (it - 1)->getEnd()));
            }
        }

        return result;
    }

    std::vector<ast::Signature> Parser::signatureListRule(const lexer::TokenTree &list) {
        std::vector<ast::Signature> result;

        auto it = TokenTreeIterator(list.tokens);
        const auto end = list.tokens.end();

        while (it) {
            if (it->isSignatureStarter()) {
                result.emplace_back(signatureRule(it));
            } else {
                auto error = errors::CompilerError(errors::ErrorCode::InvalidSignature, list.left);
                error.addLabel("expected type, tuple or function signature", *it);
                addError(std::move(error));
                recoverUntil(it, [](const auto &node) {
                    return node.isSignatureStarter() || node.isToken(lexer::TokenType::Comma);
                });
            }

            if (it) {
                if (it->isToken(lexer::TokenType::Comma)) {
                    it += 1;
                } else {
                    auto error = errors::CompilerError(errors::ErrorCode::MissingComma, list.left);
                    error.addLabel("expected a comma", *it);
                    addError(std::move(error));
                }
            }
        }

        return result;
    }

    bool Parser::recoverTopLevel(TokenTreeIterator &it) {
        auto start = it;
        while (it) {
            recoverUntil(it, [](const auto &node) {
                return node.isToken(lexer::TokenType::Semicolon)
                       || node.isTopLevelStarter()
                       || node.isModifier();
            });
            if (!tryConsumeToken(it, lexer::TokenType::Semicolon)) {
                break;
            }
        }
        return start != it;
    }

    bool Parser::recoverUntil(TokenTreeIterator &it, lexer::TokenType type) {
        return recoverUntil(it, [type](const auto &node) {
            return node.isToken(type);
        });
    }

    bool Parser::recoverUntil(TokenTreeIterator &it, std::vector<lexer::TokenType> oneOf) {
        return recoverUntil(it, [&oneOf](const auto &node) {
            return node.isToken() && std::ranges::find(oneOf, node.getToken().type) != oneOf.end();
        });
    }

    bool Parser::recoverUntil(TokenTreeIterator &it, const RecoverPredicate &predicate) {
        auto beforeRecover = it;
        while (it && !predicate(*it)) {
            it += 1;
        }

        if (beforeRecover != it) {
            addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
        return beforeRecover != it;
    }

    bool Parser::recoverUntil(TokenTreeIterator &it, const RecoverPredicate &predicate,
                              errors::ErrorContext &errCtx) {
        const auto beforeRecover = it;
        while (it && !predicate(*it)) {
            it += 1;
        }

        if (beforeRecover != it) {
            errCtx.addError(errors::CompilerError(errors::ErrorCode::UnexpectedToken, beforeRecover->getStart()));
        }
        return beforeRecover != it;
    }

    lexer::Token Parser::consumeToken(TokenTreeIterator &it, const lexer::TokenType type) const {
        COMPILER_ASSERT(it, std::format("trying to consume {} on empty iterator at {}", TokenTypeName(type),
                                        source->getLocation((it - 1)->getEnd() - source->offset)));
        COMPILER_ASSERT(it->isToken(type),
                        std::format("trying to consume {} but got {} at {}", TokenTypeName(type), it->debugString(),
                                    source->getLocation((it - 1)->getEnd() - source->offset)));

        auto token = it->getToken();
        it += 1;
        return token;
    }

    std::optional<lexer::Token> Parser::tryConsumeToken(TokenTreeIterator &it, const lexer::TokenType type) {
        if (it.isEnd() || !it->isToken(type)) {
            return std::nullopt;
        }

        auto token = it->getToken();
        it += 1;
        return token;
    }

    auto Parser::consumeTokenTree(TokenTreeIterator &it, lexer::TokenType type) -> const lexer::TokenTree & {
        COMPILER_ASSERT(it,
                        std::format("trying to consume TokenTree[{}] on empty iterator", TokenTypeName(type)));
        COMPILER_ASSERT(it->isTokenTree(type),
                        std::format("trying to consume TokenTree[{}] but got {}", TokenTypeName(type), it->debugString()
                        ));

        auto const &tree = it->getTokenTree();

        if (tree.right.isError()) {
            auto error = errors::CompilerError(errors::ErrorCode::WrongCloser, tree.right.getOrErrorToken());
            error.setNote("unclosed list, expected: " + TokenTypeStringQuoted(tree.left.expectedClosing()));
            addError(std::move(error));
        }

        it += 1;
        return tree;
    }

    std::optional<const lexer::TokenTree *> Parser::tryConsumeTokenTree(TokenTreeIterator &it,
                                                                        const lexer::TokenType type) {
        if (it.isEnd() || !it->isTokenTree(type)) {
            return std::nullopt;
        }

        auto const &tree = it->getTokenTree();

        if (tree.right.isError()) {
            auto error = errors::CompilerError(errors::ErrorCode::WrongCloser, tree.right.getOrErrorToken());
            error.setNote("unclosed list: expected " + TokenTypeStringQuoted(tree.left.expectedClosing()));
            addError(std::move(error));
        }

        it += 1;
        return &tree;
    }

}

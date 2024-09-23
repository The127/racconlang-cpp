#include <iostream>

#include "ast/AliasDeclaration.h"
#include "ast/EnumDeclaration.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/ModuleDeclaration.h"
#include "errors/ConsoleErrorHandler.h"
#include "errors/ErrorHandler.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "registry/ModuleRegistry.h"
#include "registry/Module.h"
#include "sourceMap/SourceMap.h"


int main() {
    auto directory = "demo";


    SourceMap sources{};

    const auto source = sources.addEntry("demo/test.rc");

    Lexer lexer(source);
    lexer.tokenize();


    std::cout << source->tokenTree->toString(sources, 0);

    Parser parser(source);
    auto modules = parser.parse();

    /*for (auto &module: modules) {

        std::cout << std::string(5, '\n');
        std::cout << std::string(10, '#');
        std::cout << std::string(2, '\n');

        std::cout << module.toString(sources, 1, true);
        std::cout << std::endl << std::endl;
    }*/

    ModuleRegistry moduleRegistry{};
    for (auto &moduleDecl: modules) {
        auto modulePath = moduleDecl.buildPathString();
        moduleRegistry.addModule(modulePath);
        auto &module = moduleRegistry.getModule(modulePath);

        //TODO: what to do with declarations that have the same generic parameter multiple times?

        for (auto &structDeclaration: moduleDecl.structDeclarations) {
            if (!structDeclaration.name)
                continue;

            module.addStruct(
                source,
                std::string(structDeclaration.name->name),
                structDeclaration.genericParams.size(),
                structDeclaration,
                moduleDecl.uses);
        }

        for (auto &enumDeclaration: moduleDecl.enumDeclarations) {
            if (!enumDeclaration.name)
                continue;

            module.addEnum(
                source,
                std::string(enumDeclaration.name->name),
                enumDeclaration.genericParams.size(),
                enumDeclaration,
                moduleDecl.uses);
        }

        for (auto &interfaceDeclaration: moduleDecl.interfaceDeclarations) {
            if (!interfaceDeclaration.name)
                continue;

            module.addInterface(
                source,
                std::string(interfaceDeclaration.name->name),
                interfaceDeclaration.genericParams.size(),
                interfaceDeclaration,
                moduleDecl.uses);
        }

        for (auto &aliasDeclaration: moduleDecl.aliasDeclarations) {
            if (!aliasDeclaration.name)
                continue;

            module.addAlias(
                source,
                std::string(aliasDeclaration.name->name),
                aliasDeclaration.genericParams.size(),
                aliasDeclaration,
                moduleDecl.uses);
        }
    }

    const std::unique_ptr<ErrorHandler> errorHandler = std::make_unique<ConsoleErrorHandler>(ConsoleErrorHandler());
    for (const auto &error: source->errors) {
        errorHandler->handleError(error, sources);
    }

    return 0;
}

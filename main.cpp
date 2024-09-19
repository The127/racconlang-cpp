#include <iostream>

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

    ModuleRegistry moduleRegistry{};

    SourceMap sources{};

    const auto source = sources.addEntry("demo/test.rc");

    Lexer lexer(source);
    lexer.tokenize();


    // std::cout << source->tokenTree->toString(sources, 0);

    Parser parser(source);
    parser.parse(moduleRegistry);


    const std::unique_ptr<ErrorHandler> errorHandler = std::make_unique<ConsoleErrorHandler>(ConsoleErrorHandler());
    for (const auto& error : source->errors) {
        errorHandler->handleError(error, sources);
    }

    for (const auto& [_, mod] : moduleRegistry.modules) {
        std::cout << std::string(5, '\n');
        std::cout << std::string(10, '#');
        std::cout << std::string(2, '\n');

        std::cout << "Module " << mod.name << ":" << std::endl;

        for (const auto &part: mod.parts) {

            std::cout << part.toString(sources, 1, true);
            std::cout << std::endl << std::endl;
        }
    }

    return 0;
}

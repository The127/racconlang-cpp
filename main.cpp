#include <iostream>

#include "ast/ModuleDeclaration.h"
#include "errors/ConsoleErrorHandler.h"
#include "errors/ErrorHandler.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sourceMap/SourceMap.h"


int main() {
    auto directory = "demo";

    SourceMap sources{};
    const auto source = sources.addEntry("demo/test.rc");


    Lexer lexer(source);
    lexer.tokenize();

    std::cout << source->tokenTree->toString(sources, 0);

    std::cout << std::string(5, '\n');
    std::cout << std::string(10, '#');
    std::cout << std::string(5, '\n');

    Parser parser(source);

    const auto& mods = parser.parse();
    for (const auto& mod : mods) {
        std::cout << std::string(5, '\n');
        std::cout << std::string(10, '#');
        std::cout << std::string(5, '\n');

        std::cout << mod.toString(sources, 1, true);
        std::cout << std::endl << std::endl;
    }

    const std::unique_ptr<ErrorHandler> errorHandler = std::make_unique<ConsoleErrorHandler>(ConsoleErrorHandler());
    for (const auto& error : source->errors) {
        errorHandler->handleError(error, sources);
    }

    return 0;
}

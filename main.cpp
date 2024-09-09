#include <iostream>

#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sourceMap/SourceMap.h"


int main() {
    auto directory = "demo";

    SourceMap sources;
    const auto source = sources.addEntry("demo/test.rc");

    Lexer lexer(source);
    source->tokenTree = lexer.tokenize();

    std::cout << source->tokenTree->toString(sources, 0);

    std::cout << std::string(5, '\n');
    std::cout << std::string(10, '#');
    std::cout << std::string(5, '\n');

    Parser parser(source);
    for (const auto error : source->errors) {
        std::cout << error.code << std::endl;
    }

    const auto mods = parser.parse();
    for (const auto& mod : mods) {
        std::cout << std::string(5, '\n');
        std::cout << std::string(10, '#');
        std::cout << std::string(5, '\n');

        std::cout << mod.toString(sources, 1, true);
    }

    return 0;
}

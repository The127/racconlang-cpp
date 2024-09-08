#include <iostream>

#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sourceMap/SourceMap.h"


int main() {
    auto directory = "demo";

    SourceMap sources;
    auto source = sources.addEntry("demo/test.rc");

    Lexer lexer(source);
    const auto tt = lexer.tokenize();
    std::cout << tt.toString(sources, 0);

    std::cout << std::string(5, '\n');
    std::cout << std::string(10, '#');
    std::cout << std::string(5, '\n');

    Parser parser(tt, sources);
    const auto mods = parser.parse();
    for (const auto& mod : mods) {
        std::cout << std::string(5, '\n');
        std::cout << std::string(10, '#');
        std::cout << std::string(5, '\n');

        ///std::cout << mod.toString(sources, 0);
    }

    return 0;
}

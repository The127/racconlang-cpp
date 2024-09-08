#include <iostream>

#include "lexer/Lexer.h"
#include "sourceMap/SourceMap.h"


int main() {
    auto directory = "demo";

    SourceMap sources;
    auto source = sources.addEntry("demo/test.rc");

    Lexer lexer(source);
    auto tt = lexer.tokenize();
    std::cout << tt.toString(sources, 0);


    return 0;
}

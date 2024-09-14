//
// Created by zoe on 14.09.24.
//

#include "ConsoleErrorHandler.h"
#include "ErrorCode.h"
#include "sourceMap/Source.h"
#include "sourceMap/SourceMap.h"

void ConsoleErrorHandler::handleError(const CompilerError &error, const SourceMap& sources) {
    const auto &[code, name] = ErrorInfo(error.code);
    std::cout << "[" << code << "] Error: " << name << std::endl;

    const auto& loc = sources.getLocation(error.position);
    const auto lineNum = std::format(" {} ", loc.line);
    std::cout << std::string(lineNum.size(), ' ') << "╭─[" << loc.fileName << ":" << loc.line << ":" << loc.column << "]" << std::endl;

    const auto source = sources.findSourceByPosition(error.position);
        const auto line = source->getLine(loc.line);
        std::cout << lineNum  << "│ " << line << std::endl;
        std::cout << std::string(lineNum.size(), ' ') << "┊ " << std::string(loc.column - 1, ' ') << "^" << std::endl;

    std::cout << std::string(lineNum.size(), ' ') << "╯";
    if (error.note) {
        std::cout << " Note: " << *error.note;
    }
    std::cout << std::endl;
}

ConsoleErrorHandler::~ConsoleErrorHandler() = default;

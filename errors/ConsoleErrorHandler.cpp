//
// Created by zoe on 14.09.24.
//

#include "ConsoleErrorHandler.h"
#include "ErrorCode.h"
#include "sourceMap/Source.h"
#include "sourceMap/SourceMap.h"

void ConsoleErrorHandler::handleError(const CompilerError &error, const SourceMap &sources) {
    const auto &[code, name] = ErrorInfo(error.code);
    std::cout << "[" << code << "] Error: " << name << std::endl;

    const auto &errorLocation = sources.getLocation(error.position);
    const auto errorLineText = std::format(" {} ", errorLocation.line);
    std::cout << std::string(errorLineText.size(), ' ') << "╭─[" << errorLocation.fileName << ":" << errorLocation.line
            << ":" << errorLocation.column << "]" << std::endl;

    const auto line = sources.getLine(error.position);
    std::cout << errorLineText << "│ " << line << std::endl;
    std::cout << std::string(errorLineText.size(), ' ') << "┊ " << std::string(errorLocation.column - 1, ' ') << "^" << std::endl;

    for (auto &item : error.labels) {
        COMPILER_ASSERT(item.isSimpleLabel(), "only simple labels allowed so far");
        const auto& label = item.simpleLabel();

        std::cout << std::string(errorLineText.size(), ' ') << "┊ " << label.text() << std::endl;
    }

    std::cout << std::string(errorLineText.size(), ' ') << "╯";
    if (error.note) {
        std::cout << " Note: " << *error.note;
    }
    std::cout << std::endl;
}

ConsoleErrorHandler::~ConsoleErrorHandler() = default;

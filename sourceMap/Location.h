//
// Created by zoe on 06.09.24.
//

#pragma once

#include "predeclare.h"

#include <format>
#include <cstdint>
#include <string>

class racc::sourcemap::Location {
public:
    std::string_view fileName;
    uint32_t line;
    uint32_t column;

    Location(std::string_view fileName, uint32_t line, uint32_t column);
};



template<class CharT>
struct std::formatter<racc::sourcemap::Location, CharT> {
    template <typename FormatParseContext>
    constexpr auto parse(FormatParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const racc::sourcemap::Location &loc, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{}:{}:{}", loc.fileName, loc.line, loc.column);
    }
};

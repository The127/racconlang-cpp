//
// Created by zoe on 06.09.24.
//

#pragma once

#include <fstream>
#include <memory>
#include <vector>

#include "Source.h"

class Token;

class SourceMap {
public:
    uint64_t offset{};
    std::vector<Source> entries{};

    SourceMap() = default;

    SourceMap(const SourceMap&) = delete;
    SourceMap& operator=(const SourceMap&) = delete;
    SourceMap(SourceMap&&) = default;

    Source& addEntry(const std::string &fileName);


    [[nodiscard]] Location getLocation(uint64_t position) const;
    [[nodiscard]] std::string_view getText(uint64_t start, uint64_t end) const;
    [[nodiscard]] std::string_view getText(const Token& token) const;

private:
    [[nodiscard]] const Source& findEntryByPosition(uint64_t position) const;
};

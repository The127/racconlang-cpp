//
// Created by zoe on 06.09.24.
//

#include "SourceMap.h"
#include "lexer/Token.h"

std::shared_ptr<Source> SourceMap::addEntry(const std::string &fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + fileName);
    }
    std::string text(std::istreambuf_iterator<char>(file), {});

    auto entry = std::make_shared<Source>(fileName, std::move(text), offset);
    entries.push_back(entry);

    offset += text.size();
    return entry;
}

std::shared_ptr<Source> SourceMap::findEntryByPosition(const uint64_t position) const {
    const auto it = std::lower_bound(entries.begin(), entries.end(), position,
        [](const std::shared_ptr<Source> &entry, const uint64_t pos) {
            return entry->offset <= pos;
        });
    const auto index = std::distance(entries.begin(), it) - 1;
    return entries[index];
}

Location SourceMap::getLocation(const uint64_t position) const {
    auto entry = findEntryByPosition(position);
    return entry->getLocation(position - entry->offset);
}

std::string_view SourceMap::getText(const uint64_t start, const uint64_t end) const {
    auto entry = findEntryByPosition(start);
    return entry->getText(start - entry->offset, end - entry->offset);
}


std::string_view SourceMap::getText(const Token& token) const {
    return getText(token.start, token.end);
}
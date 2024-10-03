//
// Created by zoe on 06.09.24.
//

#include "SourceMap.h"

#include "Location.h"
#include "Source.h"
#include "lexer/Token.h"

namespace racc::sourcemap {

    SourceMap::SourceMap() = default;

    SourceMap &SourceMap::operator=(SourceMap &&) noexcept = default;

    SourceMap::SourceMap(SourceMap &&) noexcept = default;

    std::shared_ptr<Source> SourceMap::addEntry(const std::string &fileName) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + fileName);
        }
        std::string text(std::istreambuf_iterator<char>(file), {});
        auto size = text.size();
        auto source = std::make_shared<Source>(fileName, std::move(text), offset);
        entries.push_back(source);

        offset += size;
        return source;
    }

    std::shared_ptr<Source> SourceMap::findSourceByPosition(const uint64_t position) const {
        const auto it = std::lower_bound(entries.begin(), entries.end(), position,
                                         [](const std::shared_ptr<Source> &entry, const uint64_t pos) {
                                             return entry->offset <= pos;
                                         });
        const auto index = std::distance(entries.begin(), it) - 1;
        return entries[index];
    }

    std::string_view SourceMap::getLine(const uint64_t position) const {
        const auto entry = findSourceByPosition(position);
        return entry->getLine(position - entry->offset);
    }

    Location SourceMap::getLocation(const uint64_t position) const {
        const auto entry = findSourceByPosition(position);
        return entry->getLocation(position - entry->offset);
    }

    std::string_view SourceMap::getText(const uint64_t start, const uint64_t end) const {
        const auto entry = findSourceByPosition(start);
        return entry->getText(start - entry->offset, end - entry->offset);
    }


    std::string_view SourceMap::getText(const lexer::Token &token) const {
        return getText(token.start, token.end);
    }

}

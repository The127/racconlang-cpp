//
// Created by zoe on 06.09.24.
//

#pragma once

#include <fstream>
#include <memory>
#include <vector>

#include "Source.h"


class SourceMap {
public:
    uint64_t offset{};
    std::vector<std::shared_ptr<Source>> entries{};

    std::shared_ptr<Source> addEntry(const std::string &fileName);

    [[nodiscard]] Location getLocation(uint64_t position) const;
    [[nodiscard]] std::string_view getText(uint64_t start, uint64_t end) const;

private:
    [[nodiscard]] std::shared_ptr<Source> findEntryByPosition(uint64_t position) const;
};

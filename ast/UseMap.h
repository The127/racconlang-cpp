//
// Created by gwendolyn on 9/26/24.
//

#pragma once

#include <map>
#include <string>
#include <optional>


class UseMap {
public:
    std::map<std::string, std::string, std::less<>> uses;

    [[nodiscard]] std::optional<std::string> lookup(std::string_view name) const;
};
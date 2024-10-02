//
// Created by zoe on 09.09.24.
//

#include "FileUses.h"
#include "errors/InternalError.h"
#include "UseMap.h"

#include <ranges>

FileUses::FileUses() = default;

FileUses::FileUses(FileUses &&other) noexcept = default;

FileUses &FileUses::operator=(FileUses &&other) noexcept = default;

FileUses::~FileUses() = default;

std::shared_ptr<UseMap> FileUses::toMap() {
    auto map = std::make_shared<UseMap>();
    for (const auto &node: uses) {
        COMPILER_ASSERT(!node.path.parts.empty(), "use path is empty");

        std::string path;
        for (auto& part : node.path.parts) {
            path += part.name;
            if (&part != &node.path.parts.back()) {
                path += "::";
            }
        }

        if (node.names.empty()) {
            auto as = node.as.and_then([](const auto &i) -> std::optional<std::string> {
                return std::string(i.name);
            }).value_or(std::string(node.path.parts.back().name));
            auto [_, success] = map->uses.emplace(as, path);
            COMPILER_ASSERT(success, "TODO: error handling");

        } else {
            COMPILER_ASSERT(!node.as, "unexpected as on use node");

            for (const auto &[nameIdent, asOpt]: node.names) {
                auto as = std::string(asOpt.value_or(nameIdent).name);
                auto name = std::string(nameIdent.name);
                auto currentPath = path;
                currentPath += "::";
                currentPath += name;
                auto [_, success] = map->uses.emplace(as, currentPath);
                COMPILER_ASSERT(success, "TODO: error handling");
            }
        }
    }
    return map;
}
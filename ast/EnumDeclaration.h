//
// Created by zoe on 07.09.24.
//

#pragma once

#include "predeclare.h"

#include "Node.h"
#include "Identifier.h"

#include <string>
#include <vector>
#include <memory>

namespace racc::ast {

    class EnumDeclaration final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        bool isPublic{};
        std::optional<Identifier> name;
        std::vector<Identifier> genericParams;
        std::vector<ConstraintDeclaration> genericConstraints;
        std::vector<EnumMemberDeclaration> memberDeclarations;

        EnumDeclaration();

        EnumDeclaration(const EnumDeclaration &) = delete;

        EnumDeclaration &operator=(const EnumDeclaration &) = delete;

        EnumDeclaration(EnumDeclaration &&) noexcept;

        EnumDeclaration &operator=(EnumDeclaration &&) noexcept;

        ~EnumDeclaration() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}
//
// Created by zoe on 07.09.24.
//


#pragma once

#include "predeclare.h"

#include "Identifier.h"
#include "Node.h"
#include "Signature.h"

#include <optional>
#include <vector>

namespace racc::ast {
    class AliasDeclaration final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        bool isPublic{};
        std::optional<Identifier> name;
        std::vector<Identifier> genericParams;
        std::vector<ConstraintDeclaration> genericConstraints;
        std::optional<Signature> signature;

        AliasDeclaration();

        AliasDeclaration(const AliasDeclaration &) = delete;

        AliasDeclaration &operator=(const AliasDeclaration &) = delete;

        AliasDeclaration(AliasDeclaration &&) noexcept;

        AliasDeclaration &operator=(AliasDeclaration &&) noexcept;

        ~AliasDeclaration() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}
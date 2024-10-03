//
// Created by zoe on 07.09.24.
//


#pragma once

#include "predeclare.h"

#include "Node.h"
#include "Identifier.h"
#include "ReturnType.h"

#include <memory>
#include <string>
#include <vector>

namespace racc::ast {
    class InterfaceMethodDeclaration final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        bool isMut{};
        std::optional<Identifier> name;
        std::vector<Identifier> genericParams;
        std::vector<ConstraintDeclaration> genericConstraints;
        std::vector<Parameter> parameters;
        std::optional<ReturnType> returnType;

        InterfaceMethodDeclaration();

        InterfaceMethodDeclaration(const InterfaceMethodDeclaration &) = delete;

        InterfaceMethodDeclaration &operator=(const InterfaceMethodDeclaration &) = delete;

        InterfaceMethodDeclaration(InterfaceMethodDeclaration &&) noexcept;

        InterfaceMethodDeclaration &operator=(InterfaceMethodDeclaration &&other) noexcept;

        ~InterfaceMethodDeclaration() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}
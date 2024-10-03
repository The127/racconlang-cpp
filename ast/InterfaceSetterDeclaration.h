//
// Created by zoe on 12.09.24.
//

#pragma once

#include "Identifier.h"
#include "Node.h"
#include "Parameter.h"

namespace racc::ast {

    class InterfaceSetterDeclaration final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        std::optional<Identifier> name;
        std::optional<Parameter> parameter;

        InterfaceSetterDeclaration();

        InterfaceSetterDeclaration(const InterfaceSetterDeclaration &) = delete;

        InterfaceSetterDeclaration &operator=(const InterfaceSetterDeclaration &) = delete;

        InterfaceSetterDeclaration(InterfaceSetterDeclaration &&) noexcept;

        InterfaceSetterDeclaration &operator=(InterfaceSetterDeclaration &&) noexcept;

        ~InterfaceSetterDeclaration() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}

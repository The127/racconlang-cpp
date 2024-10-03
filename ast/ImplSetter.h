//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"
#include "Identifier.h"
#include "Parameter.h"

namespace racc::ast {

    class ImplSetter final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        bool isPublic{};
        std::optional<Identifier> name;
        std::optional<Parameter> parameter;

        ImplSetter();

        ImplSetter(const ImplSetter &) = delete;

        ImplSetter &operator=(const ImplSetter &) = delete;

        ImplSetter(ImplSetter &&) noexcept;

        ImplSetter &operator=(ImplSetter &&) noexcept;

        ~ImplSetter() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}

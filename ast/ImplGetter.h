//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"

#include "Identifier.h"
#include "ReturnType.h"

namespace racc::ast {

    class ImplGetter final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        bool isPublic{};
        bool isMut{};
        std::optional<Identifier> name;
        std::optional<ReturnType> returnType;

        ImplGetter();

        ImplGetter(const ImplGetter &) = delete;

        ImplGetter &operator=(const ImplGetter &) = delete;

        ImplGetter(ImplGetter &&) noexcept;

        ImplGetter &operator=(ImplGetter &&) noexcept;

        ~ImplGetter() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}
#pragma once

#include "Identifier.h"
#include "Node.h"
#include "Signature.h"

#include <optional>
#include <memory>

namespace racc::ast {

    class NamelessParameter final : Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        bool isMut{};
        bool isRef{};
        Signature type;

        NamelessParameter(Signature type);

        NamelessParameter(const NamelessParameter &) = delete;

        NamelessParameter &operator=(const NamelessParameter &) = delete;

        NamelessParameter(NamelessParameter &&) noexcept;

        NamelessParameter &operator=(NamelessParameter &&) noexcept;

        ~NamelessParameter() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}
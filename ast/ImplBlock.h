//
// Created by zoe on 14.09.24.
//


#pragma once

#include "predeclare.h"

#include "Node.h"
#include "TypeSignature.h"
#include "Identifier.h"

namespace racc::ast {
    class ImplBlock final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        std::vector<Identifier> genericParams;
        std::optional<TypeSignature> interfaceName;
        std::optional<TypeSignature> structName;
        std::optional<TypeSignature> forInterfaceName;
        std::vector<ConstraintDeclaration> genericConstraints;
        std::vector<ConstructorDeclaration> constructors;
        std::vector<DestructureDeclaration> destructors;
        std::vector<ImplMethod> methods;
        std::vector<ImplSetter> setters;
        std::vector<ImplGetter> getters;


        ImplBlock();

        ImplBlock(const ImplBlock &) = delete;

        ImplBlock &operator=(const ImplBlock &) = delete;

        ImplBlock(ImplBlock &&) noexcept;

        ImplBlock &operator=(ImplBlock &&) noexcept;

        ~ImplBlock() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}
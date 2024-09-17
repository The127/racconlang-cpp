//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Identifier.h"
#include "Node.h"


class Parameter;

class ConstructorDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Parameter> parameters;

    ConstructorDeclaration();
    ConstructorDeclaration(ConstructorDeclaration &&) noexcept;
    ConstructorDeclaration &operator=(ConstructorDeclaration &&) noexcept;
    ~ConstructorDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

//
// Created by zoe on 07.09.24.
//

#pragma once

#include "predeclare.h"

#include "Identifier.h"
#include "Node.h"
#include "Signature.h"
#include "ReturnType.h"

#include <memory>
#include <string>
#include <vector>


class racc::ast::FunctionDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<Parameter> parameters;
    std::optional<ReturnType> returnType;
    //TODO: body as expression

    FunctionDeclaration();

    FunctionDeclaration(const FunctionDeclaration &) = delete;

    FunctionDeclaration &operator=(const FunctionDeclaration &) = delete;

    FunctionDeclaration(FunctionDeclaration &&) noexcept;

    FunctionDeclaration &operator=(FunctionDeclaration &&) noexcept;

    ~FunctionDeclaration() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
};
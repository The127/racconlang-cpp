//
// Created by zoe on 07.09.24.
//

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Identifier.h"
#include "Node.h"
#include "SignatureBase.h"

class Parameter;
class ConstraintDeclaration;

class FunctionDeclaration final : public Node{
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<Parameter> parameters;
    std::unique_ptr<SignatureBase> returnType;
    //TODO: body as expression

    FunctionDeclaration();
    FunctionDeclaration(FunctionDeclaration&&) noexcept;
    FunctionDeclaration& operator=(FunctionDeclaration&&) noexcept;
    ~FunctionDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

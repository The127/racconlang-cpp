//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"
#include "Identifier.h"
#include "ReturnType.h"


class Parameter;
class ConstraintDeclaration;

class ImplMethod final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    bool isStatic{};
    bool isMut{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<Parameter> parameters;
    std::optional<ReturnType> returnType;

    ImplMethod();
    ImplMethod(const ImplMethod&) = delete;
    ImplMethod& operator=(const ImplMethod&) = delete;
    ImplMethod(ImplMethod&&) noexcept;
    ImplMethod& operator=(ImplMethod&&) noexcept;
    ~ImplMethod() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

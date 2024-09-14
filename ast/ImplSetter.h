//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"


class ImplSetter final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};

    ImplSetter();
    ImplSetter(ImplSetter&&) noexcept;
    ImplSetter& operator=(ImplSetter&&) noexcept;
    ~ImplSetter() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

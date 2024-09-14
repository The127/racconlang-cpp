//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"


class ImplMethod final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    bool isStatic{};
    bool isMut{};

    ImplMethod();
    ImplMethod(ImplMethod&&) noexcept;
    ImplMethod& operator=(ImplMethod&&) noexcept;
    ~ImplMethod() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

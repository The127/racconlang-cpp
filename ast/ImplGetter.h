//
// Created by zoe on 14.09.24.
//


#pragma once
#include "Node.h"


class ImplGetter final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    bool isMut{};

    ImplGetter();
    ImplGetter(ImplGetter&&) noexcept;
    ImplGetter& operator=(ImplGetter&&) noexcept;
    ~ImplGetter() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};

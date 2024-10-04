//
// Created by zoe on 09.09.24.
//


#pragma once

#include "predeclare.h"

#include "UseNode.h"
#include "Identifier.h"
#include "Path.h"

#include <map>

class racc::ast::FileUses {
public:
    std::vector<UseNode> uses;

    FileUses();

    FileUses(const FileUses &) = delete;

    FileUses &operator=(const FileUses &) = delete;

    FileUses(FileUses &&other) noexcept;

    FileUses &operator=(FileUses &&other) noexcept;

    ~FileUses();

    std::shared_ptr<UseMap> toMap();
};

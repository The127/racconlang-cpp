//
// Created by zoe on 14.09.24.
//


#pragma once
#include <variant>

#include "ErrorLabelGroup.h"
#include "SimpleErrorLabel.h"


class ErrorLabel {
public:
    explicit(false) ErrorLabel(SimpleErrorLabel label);
    explicit(false) ErrorLabel(ErrorLabelGroup group);

    [[nodiscard]] bool isSimpleLabel() const;
    [[nodiscard]] bool isLabelGroup() const;

    [[nodiscard]] const SimpleErrorLabel &simpleLabel() const;
    [[nodiscard]] const ErrorLabelGroup &labelGroup() const;
private:
    std::variant<SimpleErrorLabel, ErrorLabelGroup> item;
};


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
private:
    std::variant<SimpleErrorLabel, ErrorLabelGroup> item;
};


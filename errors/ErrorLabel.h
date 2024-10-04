//
// Created by zoe on 14.09.24.
//


#pragma once

#include "predeclare.h"

#include "ErrorLabelGroup.h"
#include "SimpleErrorLabel.h"

#include <variant>

    class racc::errors::ErrorLabel {
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

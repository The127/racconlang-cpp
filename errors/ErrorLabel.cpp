//
// Created by zoe on 14.09.24.
//

#include "ErrorLabel.h"

ErrorLabel::ErrorLabel(SimpleErrorLabel label)
    : item(std::move(label)) {
}

ErrorLabel::ErrorLabel(ErrorLabelGroup group)
    : item(std::move(group)) {
}

bool ErrorLabel::isSimpleLabel() const {
    return std::holds_alternative<SimpleErrorLabel>(item);
}

bool ErrorLabel::isLabelGroup() const {
    return std::holds_alternative<ErrorLabelGroup>(item);
}

const SimpleErrorLabel & ErrorLabel::simpleLabel() const {
    return std::get<SimpleErrorLabel>(item);
}

const ErrorLabelGroup & ErrorLabel::labelGroup() const {
    return std::get<ErrorLabelGroup>(item);
}

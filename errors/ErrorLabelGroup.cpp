//
// Created by zoe on 14.09.24.
//

#include "ErrorLabelGroup.h"

#include "InternalError.h"

namespace racc::errors {

    ErrorLabelGroup::ErrorLabelGroup(std::string text, uint64_t start, uint64_t end)
            : _text(std::move(text)),
              _start(start),
              _end(end) {
    }

    void ErrorLabelGroup::addLabel(SimpleErrorLabel label) {
        DEBUG_ASSERT(label.start() >= _start && label.end() <= _end, "child label has is outside of label group");
        labels.emplace_back(std::move(label));
    }

    uint64_t ErrorLabelGroup::start() const {
        return _start;
    }

    uint64_t ErrorLabelGroup::end() const {
        return _end;
    }

    std::string_view ErrorLabelGroup::text() const {
        return _text;
    }

}

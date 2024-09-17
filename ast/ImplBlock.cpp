//
// Created by zoe on 14.09.24.
//

#include "ImplBlock.h"

#include "utils/NodeUtils.h"
#include "ConstructorDeclaration.h"
#include "ImplSetter.h"
#include "ImplGetter.h"
#include "ImplMethod.h"
#include "DestructureDeclaration.h"
#include "ConstraintDeclaration.h"


ImplBlock::ImplBlock() = default;
ImplBlock::ImplBlock(ImplBlock &&) noexcept = default;
ImplBlock & ImplBlock::operator=(ImplBlock &&) noexcept = default;
ImplBlock::~ImplBlock() = default;

uint64_t ImplBlock::start() const {
    return startPos;
}

uint64_t ImplBlock::end() const {
    return endPos;
}

std::string ImplBlock::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ImplBlock", verbose) + "{\n";

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}

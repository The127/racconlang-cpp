//
// Created by zoe on 07.09.24.
//

#pragma once

#include "GenericConstraint.h"
#include "TypeSignature.h"


class InterfaceConstraint : GenericConstraint {
public:
    TypeSignature typeSignature;
};

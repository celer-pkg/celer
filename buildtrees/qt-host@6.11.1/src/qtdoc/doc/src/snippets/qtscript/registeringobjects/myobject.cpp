// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "myobject.h"

MyObject::MyObject()
{
}

int MyObject::calculate(int value) const
{
    int total = 0;
    for (int i = 0; i <= value; ++i)
        total += i;
    return total;
}

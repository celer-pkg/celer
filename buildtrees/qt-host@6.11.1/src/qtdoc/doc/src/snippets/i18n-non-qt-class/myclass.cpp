// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <iostream>
#include "myclass.h"

MyClass::MyClass()
{
    std::cout << tr("Hello Qt!\n").toLocal8Bit().constData();
}

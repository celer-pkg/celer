// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MYCLASS_H
#define MYCLASS_H

#include <QCoreApplication>

//! [0]
class MyClass
{
    Q_DECLARE_TR_FUNCTIONS(MyClass)

public:
    MyClass();
//! [0]
    /* ... */
//! [1]
};
//! [1]

#endif

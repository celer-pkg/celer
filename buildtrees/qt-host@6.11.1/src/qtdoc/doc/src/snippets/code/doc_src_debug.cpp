// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
qDebug() << "Widget" << widget << "at position" << widget->pos();
//! [0]


//! [1]
char *alloc(int size)
{
    Q_ASSERT(size > 0);
    char *ptr = new char[size];
    Q_CHECK_PTR(ptr);
    return ptr;
}
//! [1]


//! [2]
char *alloc(int size)
{
    char *ptr;
    Q_CHECK_PTR(ptr = new char[size]);  // WRONG
    return ptr;
}
//! [2]

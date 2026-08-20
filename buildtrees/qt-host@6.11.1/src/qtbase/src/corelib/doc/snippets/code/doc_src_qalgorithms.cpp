// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QList>
#include <QColor>

void example()
{
    //! [1]
    QList<QColor *> list;
    list.append(new QColor(Qt::blue));
    list.append(new QColor(Qt::yellow));

    qDeleteAll(list.begin(), list.end());
    list.clear();
    //! [1]
}

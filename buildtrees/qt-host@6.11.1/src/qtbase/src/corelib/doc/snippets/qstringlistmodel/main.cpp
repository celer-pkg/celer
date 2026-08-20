// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtCore>

void example()
{
//! [0]
    QStringListModel *model = new QStringListModel();
    QStringList list;
    list << "a" << "b" << "c";
    model->setStringList(list);
//! [0]
}

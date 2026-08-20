// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QStringList>
#include <QCollator>

//! [0]
QStringList sortedStrings(QStringList seq)
{
    QCollator order;
    std::sort(seq.begin(), seq.end(), order);
    return seq;
}
//! [0]

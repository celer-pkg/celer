// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QItemSelection>
#include <QModelIndex>

void examples()
{
    QModelIndex topLeft;
    QModelIndex bottomRight;

    {
        //! [0]
        QItemSelection *selection = new QItemSelection(topLeft, bottomRight);
        //! [0]
    }

    {
        //! [1]
        QItemSelection *selection = new QItemSelection();
        //...
        selection->select(topLeft, bottomRight);
        //! [1]
    }
}

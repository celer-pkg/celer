// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

void examples()
{
    QModelIndex index;

    //! [0]
    const QAbstractItemModel *model = index.model();
    //! [0]

    int row, column;

    {
        //! [1]
        QModelIndex index = model->index(row, column /*...*/);
        //! [1]
    }

    {
        //! [2]
        QModelIndex indexA = model->index(0, 0, QModelIndex());
        QModelIndex indexB = model->index(1, 1, QModelIndex());
        QModelIndex indexC = model->index(2, 1, QModelIndex());
        //! [2]
    }

    {
        QModelIndex parent;

        //! [3]
        QModelIndex index = model->index(row, column, parent);
        //! [3]
    }

    {
        //! [4]
        QModelIndex indexA = model->index(0, 0, QModelIndex());
        QModelIndex indexC = model->index(2, 1, QModelIndex());
        //! [4]
    }

    {
        QModelIndex indexA;

        //! [5]
        QModelIndex indexB = model->index(1, 0, indexA);
        //! [5]
    }

    {
        QModelIndex index;
        int role = Qt::DisplayRole;

        //! [6]
        QVariant value = model->data(index, role);
        //! [6]
    }
}

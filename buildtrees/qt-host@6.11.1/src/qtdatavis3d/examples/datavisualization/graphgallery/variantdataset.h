// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VARIANTDATASET_H
#define VARIANTDATASET_H

#include <QtCore/qvariantlist.h>

//! [0]
using VariantDataItem = QVariantList;
using VariantDataItemList = QList<VariantDataItem *>;
//! [0]

class VariantDataSet : public QObject
{
    Q_OBJECT

public:
    explicit VariantDataSet();
    ~VariantDataSet();

    //! [1]
    void clear();

    int addItem(VariantDataItem *item);
    int addItems(VariantDataItemList *itemList);

    const VariantDataItemList &itemList() const;

Q_SIGNALS:
    void itemsAdded(int index, int count);
    void dataCleared();
    //! [1]

private:
    VariantDataItemList m_variantData;

    Q_DISABLE_COPY(VariantDataSet)
};

#endif

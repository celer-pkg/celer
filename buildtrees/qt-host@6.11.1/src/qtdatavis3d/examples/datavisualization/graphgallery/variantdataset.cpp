// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "variantdataset.h"

VariantDataSet::VariantDataSet() = default;

VariantDataSet::~VariantDataSet()
{
    clear();
}

void VariantDataSet::clear()
{
    for (VariantDataItem *item : m_variantData) {
        item->clear();
        delete item;
    }
    m_variantData.clear();
    emit dataCleared();
}

int VariantDataSet::addItem(VariantDataItem *item)
{
    m_variantData.append(item);
    int addIndex =  m_variantData.size();

    emit itemsAdded(addIndex, 1);
    return addIndex;
}

int VariantDataSet::addItems(VariantDataItemList *itemList)
{
    int newCount = itemList->size();
    int addIndex = m_variantData.size();
    m_variantData.append(*itemList);
    delete itemList;
    emit itemsAdded(addIndex, newCount);
    return addIndex;
}

const VariantDataItemList &VariantDataSet::itemList() const
{
    return m_variantData;
}

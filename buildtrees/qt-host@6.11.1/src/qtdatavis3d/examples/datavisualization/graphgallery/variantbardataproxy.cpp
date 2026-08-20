// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "variantbardataproxy.h"

VariantBarDataProxy::VariantBarDataProxy() = default;

VariantBarDataProxy::~VariantBarDataProxy()
{
    delete m_dataSet;
}

void VariantBarDataProxy::setDataSet(VariantDataSet *newSet)
{
    if (!m_dataSet.isNull())
        QObject::disconnect(m_dataSet.data(), nullptr, this, nullptr);

    m_dataSet = newSet;

    if (!m_dataSet.isNull()) {
        QObject::connect(m_dataSet.data(), &VariantDataSet::itemsAdded, this,
                         &VariantBarDataProxy::handleItemsAdded);
        QObject::connect(m_dataSet.data(), &VariantDataSet::dataCleared, this,
                         &VariantBarDataProxy::handleDataCleared);
    }
    resolveDataSet();
}

VariantDataSet *VariantBarDataProxy::dataSet()
{
    return m_dataSet.data();
}

void VariantBarDataProxy::setMapping(VariantBarDataMapping *mapping)
{
    if (!m_mapping.isNull()) {
        QObject::disconnect(m_mapping.data(), &VariantBarDataMapping::mappingChanged, this,
                            &VariantBarDataProxy::handleMappingChanged);
    }

    m_mapping = mapping;

    if (!m_mapping.isNull()) {
        QObject::connect(m_mapping.data(), &VariantBarDataMapping::mappingChanged, this,
                         &VariantBarDataProxy::handleMappingChanged);
    }

    resolveDataSet();
}

VariantBarDataMapping *VariantBarDataProxy::mapping()
{
    return m_mapping.data();
}

void VariantBarDataProxy::handleItemsAdded(int index, int count)
{
    Q_UNUSED(index);
    Q_UNUSED(count);

    // Resolve new items
    resolveDataSet();
}

void VariantBarDataProxy::handleDataCleared()
{
    // Data cleared, reset array
    resetArray(nullptr);
}

void VariantBarDataProxy::handleMappingChanged()
{
    resolveDataSet();
}

// Resolve entire dataset into QBarDataArray.
//! [0]
void VariantBarDataProxy::resolveDataSet()
{
    // If we have no data or mapping, or the categories are not defined, simply clear the array
    if (m_dataSet.isNull() || m_mapping.isNull() || !m_mapping->rowCategories().size()
            || !m_mapping->columnCategories().size()) {
        resetArray(nullptr);
        return;
    }
    const VariantDataItemList &itemList = m_dataSet->itemList();

    int rowIndex = m_mapping->rowIndex();
    int columnIndex = m_mapping->columnIndex();
    int valueIndex = m_mapping->valueIndex();
    const QStringList &rowList = m_mapping->rowCategories();
    const QStringList &columnList = m_mapping->columnCategories();

    // Sort values into rows and columns
    using ColumnValueMap = QHash<QString, float>;
    QHash <QString, ColumnValueMap> itemValueMap;
    for (const VariantDataItem *item : itemList) {
        itemValueMap[item->at(rowIndex).toString()][item->at(columnIndex).toString()]
                = item->at(valueIndex).toReal();
    }

    // Create a new data array in format the parent class understands
    auto *newProxyArray = new QBarDataArray;
    for (const QString &rowKey : rowList) {
        auto *newProxyRow = new QBarDataRow(columnList.size());
        for (qsizetype i = 0; i < columnList.size(); ++i)
            (*newProxyRow)[i].setValue(itemValueMap[rowKey][columnList.at(i)]);
        newProxyArray->append(newProxyRow);
    }

    // Finally, reset the data array in the parent class
    resetArray(newProxyArray);
}
//! [0]

// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "variantbardatamapping.h"

VariantBarDataMapping::VariantBarDataMapping(int rowIndex, int columnIndex, int valueIndex,
                                             const QStringList &rowCategories,
                                             const QStringList &columnCategories)
    : QObject(nullptr)
{
    m_rowIndex = rowIndex;
    m_columnIndex = columnIndex;
    m_valueIndex = valueIndex;
    m_rowCategories = rowCategories;
    m_columnCategories = columnCategories;
}

VariantBarDataMapping::~VariantBarDataMapping() = default;

void VariantBarDataMapping::setRowIndex(int index)
{
    m_rowIndex = index;
    emit mappingChanged();
}

int VariantBarDataMapping::rowIndex() const
{
    return m_rowIndex;
}

void VariantBarDataMapping::setColumnIndex(int index)
{
    m_columnIndex = index;
    emit mappingChanged();
}

int VariantBarDataMapping::columnIndex() const
{
    return m_columnIndex;
}

void VariantBarDataMapping::setValueIndex(int index)
{
    m_valueIndex = index;
    emit mappingChanged();
}

int VariantBarDataMapping::valueIndex() const
{
    return m_valueIndex;
}

void VariantBarDataMapping::setRowCategories(const QStringList &categories)
{
    m_rowCategories = categories;
    emit mappingChanged();
}

const QStringList &VariantBarDataMapping::rowCategories() const
{
    return m_rowCategories;
}

void VariantBarDataMapping::setColumnCategories(const QStringList &categories)
{
    m_columnCategories = categories;
    emit mappingChanged();
}

const QStringList &VariantBarDataMapping::columnCategories() const
{
    return m_columnCategories;
}

void VariantBarDataMapping::remap(int rowIndex, int columnIndex, int valueIndex,
                                  const QStringList &rowCategories,
                                  const QStringList &columnCategories)
{
    m_rowIndex = rowIndex;
    m_columnIndex = columnIndex;
    m_valueIndex = valueIndex;
    m_rowCategories = rowCategories;
    m_columnCategories = columnCategories;
    emit mappingChanged();
}

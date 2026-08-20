// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VARIANTBARDATAMAPPING_H
#define VARIANTBARDATAMAPPING_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

class VariantBarDataMapping : public QObject
{
    Q_OBJECT
    //! [0]
    Q_PROPERTY(int rowIndex READ rowIndex WRITE setRowIndex NOTIFY rowIndexChanged)
    Q_PROPERTY(int columnIndex READ columnIndex WRITE setColumnIndex NOTIFY columnIndexChanged)
    Q_PROPERTY(int valueIndex READ valueIndex WRITE setValueIndex NOTIFY valueIndexChanged)
    Q_PROPERTY(QStringList rowCategories READ rowCategories WRITE setRowCategories NOTIFY rowCategoriesChanged)
    Q_PROPERTY(QStringList columnCategories READ columnCategories WRITE setColumnCategories NOTIFY columnCategoriesChanged)
    //! [0]
public:
    //! [1]
    explicit VariantBarDataMapping(int rowIndex, int columnIndex, int valueIndex,
                                   const QStringList &rowCategories,
                                   const QStringList &columnCategories);
    //! [1]
    virtual ~VariantBarDataMapping();

    void setRowIndex(int index);
    int rowIndex() const;
    void setColumnIndex(int index);
    int columnIndex() const;
    void setValueIndex(int index);
    int valueIndex() const;

    void setRowCategories(const QStringList &categories);
    const QStringList &rowCategories() const;
    void setColumnCategories(const QStringList &categories);
    const QStringList &columnCategories() const;

    //! [2]
    void remap(int rowIndex, int columnIndex, int valueIndex,
               const QStringList &rowCategories,
               const QStringList &columnCategories);
    //! [2]
Q_SIGNALS:
    void rowIndexChanged();
    void columnIndexChanged();
    void valueIndexChanged();
    void rowCategoriesChanged();
    void columnCategoriesChanged();
    //! [3]
    void mappingChanged();
    //! [3]

private:
    // Indexes of the mapped items in the VariantDataItem
    int m_rowIndex = 0;
    int m_columnIndex = 1;
    int m_valueIndex = 2;

    // For row/column items, sort items into these categories. Other categories are ignored.
    QStringList m_rowCategories = {};
    QStringList m_columnCategories = {};
};

#endif

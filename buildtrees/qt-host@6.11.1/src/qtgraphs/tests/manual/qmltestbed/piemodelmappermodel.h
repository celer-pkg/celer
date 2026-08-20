// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef PIEMODELMAPPERMODEL_H
#define PIEMODELMAPPERMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtQmlIntegration/qqmlintegration.h>

class PieModelMapperModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    PieModelMapperModel();
    ~PieModelMapperModel() override;

    enum MyRoles { background = Qt::UserRole + 1 };

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE void addMapping(QColor color, int colStart, int rowStart, int colEnd, int rowEnd);
    Q_INVOKABLE void clearMapping() { m_mapping.clear(); }
    Q_INVOKABLE void startAddMapping() { beginResetModel(); }
    Q_INVOKABLE void endAddMapping() { endResetModel(); }

private:
    QList<QList<QString>> m_data;
    QMultiHash<QString, QRect> m_mapping;
    int m_columnCount;
    int m_rowCount;
};

#endif // PIEMODELMAPPERMODEL_H

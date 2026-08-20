// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#ifndef BARMODELMAPPERMODEL_H
#define BARMODELMAPPERMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtGui/qcolor.h>
#include <QtQmlIntegration/qqmlintegration.h>

class BarModelMapperModel : public QAbstractTableModel
{
    Q_OBJECT

    QML_ELEMENT
public:
    explicit BarModelMapperModel(QObject *parent = nullptr);
    ~BarModelMapperModel() override;

    enum MyRoles { background = Qt::UserRole + 1 };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Q_INVOKABLE void addMapping(QColor color, int colStart, int rowStart, int colEnd, int rowEnd);
    Q_INVOKABLE void clearMapping() { m_mapping.clear(); }
    Q_INVOKABLE void startAddMapping() { beginResetModel(); }
    Q_INVOKABLE void endAddMapping() { endResetModel(); }
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<QList<qreal> *> m_data;
    QMultiHash<QString, QRect> m_mapping;
    int m_columnCount;
    int m_rowCount;

    // QAbstractItemModel interface
};

#endif // BARMODELMAPPERMODEL_H

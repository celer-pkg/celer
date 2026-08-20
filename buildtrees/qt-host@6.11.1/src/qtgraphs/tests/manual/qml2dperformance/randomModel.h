// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef RANDOMMODEL_H
#define RANDOMMODEL_H

#include <qabstractitemmodel.h>
#include <qtmetamacros.h>
#include <QtQmlIntegration/qqmlintegration.h>

class RandomModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount WRITE setRowCount NOTIFY rowCountChanged)

public:
    enum Roles {
        XRole = Qt::DisplayRole + 1,
        YRole,
    };
    RandomModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void setRowCount(int rowCount);
    int columnCount(const QModelIndex &parent) const override;

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void nextCache();

Q_SIGNALS:
    void rowCountChanged(int rowCount);

private:
    void generateCaches();
    void clearData();

    int m_cacheCount = 60;
    int m_columnCount;
    int m_rowCount;
    int m_currentCache = 0;
    QVector<QList<QList<qreal>>> m_data;
};

#endif // RANDOMMODEL_H

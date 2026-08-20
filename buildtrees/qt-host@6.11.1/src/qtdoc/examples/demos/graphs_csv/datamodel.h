// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DATAMODEL_H
#define DATAMODEL_H
#include <QAbstractItemModel>
#include <QtQml/qqml.h>
#include <QList>
#include <qtpreprocessorsupport.h>

class DataModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CsvDataModel)

public:
    explicit DataModel(QObject *parent = nullptr);
    ~DataModel() override;

    enum CustomRoles { Background = Qt::UserRole + 1 };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
      Q_UNUSED(parent)
      return m_csvData.count();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return QAbstractItemModel::flags(index);
    }

    Q_INVOKABLE void readCsv(const QUrl &csvFile);

private:
    QList<QList<QVariant>> m_csvData;
    QString m_csvFile;
};

#endif

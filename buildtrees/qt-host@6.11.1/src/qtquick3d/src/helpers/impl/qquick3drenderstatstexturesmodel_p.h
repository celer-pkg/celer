// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DRENDERSTATSTEXTURESMODEL_H
#define QQUICK3DRENDERSTATSTEXTURESMODEL_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/QAbstractTableModel>
#include <QtCore/QObject>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuick3DRenderStatsTexturesModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QString textureData READ textureData WRITE setTextureData NOTIFY textureDataChanged)
    QML_NAMED_ELEMENT(RenderStatsTexturesModel)

public:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const QString &textureData() const;

public Q_SLOTS:
    void setTextureData(const QString &newTextureData);

Q_SIGNALS:
    void textureDataChanged();

private:
    struct Data {
        QString name;
        QString size;
        QString format;
        quint32 mipLevels;
        QString flags;
    };
    QVector<Data> m_data;
    QString m_textureData;
};

QT_END_NAMESPACE

#endif // QQUICK3DRENDERSTATSTEXTURESMODEL_H

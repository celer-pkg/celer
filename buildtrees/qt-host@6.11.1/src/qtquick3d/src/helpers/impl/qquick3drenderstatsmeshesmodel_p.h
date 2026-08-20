// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DRENDERSTATSMESHESMODEL_H
#define QQUICK3DRENDERSTATSMESHESMODEL_H

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

class QQuick3DRenderStatsMeshesModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QString meshData READ meshData WRITE setMeshData NOTIFY meshDataChanged)
    QML_NAMED_ELEMENT(RenderStatsMeshesModel)

public:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const QString &meshData() const;

public Q_SLOTS:
    void setMeshData(const QString &newMeshData);

Q_SIGNALS:
    void meshDataChanged();

private:
    struct Data {
        QString name;
        quint64 submeshes;
        quint64 vertices;
        quint64 vertexBufferSize;
        quint64 indexBufferSize;
    };
    QVector<Data> m_data;
    QString m_meshData;
};

QT_END_NAMESPACE

#endif // QQUICK3DRENDERSTATSMESHESMODEL_H

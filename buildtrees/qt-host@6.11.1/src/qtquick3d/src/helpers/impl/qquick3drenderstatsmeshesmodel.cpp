// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3drenderstatsmeshesmodel_p.h"

#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

QHash<int, QByteArray> QQuick3DRenderStatsMeshesModel::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}

int QQuick3DRenderStatsMeshesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.count();
}

int QQuick3DRenderStatsMeshesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant QQuick3DRenderStatsMeshesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const uint row = index.row();
    const uint column = index.column();

    if (role == Qt::DisplayRole) {
        // Name 0
        if (column == 0)
            return m_data[row].name;
        // Submeshes 1
        if (column == 1)
            return m_data[row].submeshes;
        // Vertices 2
        if (column == 2)
            return m_data[row].vertices;
        // Vertex Buffer Size 3
        if (column == 3)
            return m_data[row].vertexBufferSize;
        // Index Buffer Size 4
        if (column == 4)
            return m_data[row].indexBufferSize;
    }

    return QVariant();
}

QVariant QQuick3DRenderStatsMeshesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section > 5)
        return QVariant();

    switch (section) {
    case 0:
        return QStringLiteral("Name");
    case 1:
        return QStringLiteral("Submeshes");
    case 2:
        return QStringLiteral("Vertices");
    case 3:
        return QStringLiteral("VBuf Size");
    case 4:
        return QStringLiteral("IBuf Size");
    default:
        Q_UNREACHABLE();
        return QVariant();
    }
}

const QString &QQuick3DRenderStatsMeshesModel::meshData() const
{
    return m_meshData;
}

void QQuick3DRenderStatsMeshesModel::setMeshData(const QString &newMeshData)
{
    if (m_meshData == newMeshData)
        return;

    m_meshData = newMeshData;
    emit meshDataChanged();

    // newMeshData is just a markdown table...
    QVector<Data> newData;
    if (!m_meshData.isEmpty()) {
        auto lines = m_meshData.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);
        if (lines.size() > 2) {
            for (qsizetype i = 2; i < lines.size(); ++i) {
                const auto &line = lines.at(i);
                auto fields = line.split(QLatin1Char('|'), Qt::SkipEmptyParts);
                if (fields.size() != 5)
                    continue;
                Data data;
                bool isUInt64 = false;
                data.name = fields[0];
                data.submeshes = fields[1].toULongLong(&isUInt64);
                if (!isUInt64)
                    continue;
                data.vertices = fields[2].toULongLong(&isUInt64);
                if (!isUInt64)
                    continue;
                data.vertexBufferSize = fields[3].toULongLong(&isUInt64);
                if (!isUInt64)
                    continue;
                data.indexBufferSize = fields[4].toULongLong(&isUInt64);
                if (!isUInt64)
                    continue;
                newData.append(data);
            }
        }
    }

    // update the model
    beginResetModel();
    m_data = newData;
    endResetModel();
}

QT_END_NAMESPACE

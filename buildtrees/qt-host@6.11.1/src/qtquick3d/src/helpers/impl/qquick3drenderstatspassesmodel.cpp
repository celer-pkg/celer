// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3drenderstatspassesmodel_p.h"

#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

QHash<int, QByteArray> QQuick3DRenderStatsPassesModel::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}

int QQuick3DRenderStatsPassesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.count();
}

int QQuick3DRenderStatsPassesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant QQuick3DRenderStatsPassesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const uint row = index.row();
    const uint column = index.column();

    if (role == Qt::DisplayRole) {
        // Name 0
        if (column == 0)
            return m_data[row].name;
        // Size 1
        if (column == 1)
            return m_data[row].size;
        // Vertices 2
        if (column == 2)
            return m_data[row].vertices;
        // DrawCalls 3
        if (column == 3)
            return m_data[row].drawCalls;
    }

    return QVariant();
}

QVariant QQuick3DRenderStatsPassesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section > 4)
        return QVariant();

    switch (section) {
    case 0:
        return QStringLiteral("Name");
    case 1:
        return QStringLiteral("Size");
    case 2:
        return QStringLiteral("Vertices");
    case 3:
        return QStringLiteral("Draw Calls");
    default:
        Q_UNREACHABLE();
        return QVariant();
    }
}

const QString &QQuick3DRenderStatsPassesModel::passData() const
{
    return m_passData;
}

void QQuick3DRenderStatsPassesModel::setPassData(const QString &newPassData)
{
    if (m_passData == newPassData)
        return;

    m_passData = newPassData;
    emit passDataChanged();

    // newPassData is just a markdown table...
    QVector<Data> newData;
    if (!m_passData.isEmpty()) {
        auto lines = m_passData.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);
        if (lines.size() > 2) {
            for (qsizetype i = 2; i < lines.size(); ++i) {
                const auto &line = lines.at(i);
                auto fields = line.split(QLatin1Char('|'), Qt::SkipEmptyParts);
                if (fields.size() != 4)
                    continue;
                Data data;
                bool isUInt64 = false;
                bool isUint32 = false;
                data.name = fields[0];
                data.size = fields[1];
                data.vertices = fields[2].toULongLong(&isUInt64);
                if (!isUInt64)
                    continue;
                data.drawCalls = fields[3].toULong(&isUint32);
                if (!isUint32)
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


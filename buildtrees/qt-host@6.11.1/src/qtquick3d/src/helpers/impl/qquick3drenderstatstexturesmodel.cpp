// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3drenderstatstexturesmodel_p.h"

#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

QHash<int, QByteArray> QQuick3DRenderStatsTexturesModel::roleNames() const
{
    return { {Qt::DisplayRole, "display"} };
}

int QQuick3DRenderStatsTexturesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.count();
}

int QQuick3DRenderStatsTexturesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant QQuick3DRenderStatsTexturesModel::data(const QModelIndex &index, int role) const
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
        // Format 2
        if (column == 2)
            return m_data[row].format;
        // Mip Levels 3
        if (column == 3)
            return m_data[row].mipLevels;
        // Flags 4
        if (column == 4)
            return m_data[row].flags;
    }

    return QVariant();
}

QVariant QQuick3DRenderStatsTexturesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section > 5)
        return QVariant();

    switch (section) {
    case 0:
        return QStringLiteral("Name");
    case 1:
        return QStringLiteral("Size");
    case 2:
        return QStringLiteral("Format");
    case 3:
        return QStringLiteral("Mip Levels");
    case 4:
        return QStringLiteral("Flags");
    default:
        Q_UNREACHABLE();
        return QVariant();
    }
}

const QString &QQuick3DRenderStatsTexturesModel::textureData() const
{
    return m_textureData;
}

void QQuick3DRenderStatsTexturesModel::setTextureData(const QString &newTextureData)
{
    if (m_textureData == newTextureData)
        return;

    m_textureData = newTextureData;
    emit textureDataChanged();

    // newTextureData is just a markdown table...
    QVector<Data> newData;
    if (!m_textureData.isEmpty()) {
        auto lines = m_textureData.split(QRegularExpression(QStringLiteral("[\r\n]")), Qt::SkipEmptyParts);
        if (lines.size() > 2) {
            for (qsizetype i = 2; i < lines.size(); ++i) {
                const auto &line = lines.at(i);
                auto fields = line.split(QLatin1Char('|'), Qt::SkipEmptyParts);
                if (fields.size() < 4) // flags field can be empty
                    continue;
                Data data;
                bool isUInt32 = false;
                data.name = fields[0];
                data.size = fields[1];
                data.format = fields[2];
                data.mipLevels = fields[3].toULong(&isUInt32);
                if (!isUInt32)
                    continue;
                if (fields.size() == 5)
                    data.flags = fields[4];
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

// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSURFACEDATAITEM_H
#define QTGRAPHS_QSURFACEDATAITEM_H

#include <QtGraphs/qgraphsglobal.h>
#include <QtGui/qvector3d.h>

QT_BEGIN_NAMESPACE

class QSurfaceDataItem
{
public:
    constexpr QSurfaceDataItem() noexcept = default;
    explicit constexpr QSurfaceDataItem(QVector3D position) noexcept
        : m_position(position)
    {}
    constexpr QSurfaceDataItem(float x, float y, float z) noexcept
        : m_position(QVector3D(x, y, z))
    {}

    constexpr void setPosition(QVector3D pos) noexcept { m_position = pos; }
    constexpr QVector3D position() const noexcept { return m_position; }
    constexpr void setX(float value) noexcept { m_position.setX(value); }
    constexpr void setY(float value) noexcept { m_position.setY(value); }
    constexpr void setZ(float value) noexcept { m_position.setZ(value); }
    constexpr float x() const noexcept { return m_position.x(); }
    constexpr float y() const noexcept { return m_position.y(); }
    constexpr float z() const noexcept { return m_position.z(); }

private:
    QVector3D m_position = {};
};

QT_END_NAMESPACE

#endif

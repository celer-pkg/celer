// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSCATTERDATAITEM_H
#define QTGRAPHS_QSCATTERDATAITEM_H

#include <QtGraphs/qgraphsglobal.h>
#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

class QScatterDataItem
{
public:
    QScatterDataItem() noexcept = default;
    explicit QScatterDataItem(QVector3D position) noexcept
        : m_position(position)
    {}
    explicit QScatterDataItem(float x, float y, float z) noexcept
        : m_position(QVector3D(x, y, z))
    {}
    explicit QScatterDataItem(QVector3D position, const QQuaternion &rotation) noexcept
        : m_position(position)
        , m_rotation(rotation)
    {}

    void setPosition(QVector3D pos) noexcept { m_position = pos; }
    QVector3D position() const noexcept { return m_position; }
    void setRotation(const QQuaternion &rot) noexcept { m_rotation = rot; }
    QQuaternion rotation() const { return m_rotation; }

    void setX(float value) noexcept { m_position.setX(value); }
    void setY(float value) noexcept { m_position.setY(value); }
    void setZ(float value) noexcept { m_position.setZ(value); }
    float x() const noexcept { return m_position.x(); }
    float y() const noexcept { return m_position.y(); }
    float z() const noexcept { return m_position.z(); }

private:
    QVector3D m_position = {};
    QQuaternion m_rotation = {};
    Q_DECL_UNUSED_MEMBER quintptr reserved = 0;
};

QT_END_NAMESPACE

#endif

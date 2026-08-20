// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QSCATTERDATAITEM_H
#define QTDATAVIS3D_QSCATTERDATAITEM_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtGui/QQuaternion>

QT_BEGIN_NAMESPACE

class QScatterDataItemPrivate;

class Q_DATAVISUALIZATION_EXPORT QScatterDataItem
{
public:
    QScatterDataItem();
    QScatterDataItem(const QVector3D &position);
    QScatterDataItem(const QVector3D &position, const QQuaternion &rotation);
    QScatterDataItem(const QScatterDataItem &other);
    ~QScatterDataItem();

    QScatterDataItem &operator=(const QScatterDataItem &other);

    inline void setPosition(const QVector3D &pos) { m_position = pos; }
    inline QVector3D position() const { return m_position; }
    inline void setRotation(const QQuaternion &rot) { m_rotation = rot; }
    inline QQuaternion rotation() const { return m_rotation; }
    inline void setX(float value) { m_position.setX(value); }
    inline void setY(float value) { m_position.setY(value); }
    inline void setZ(float value) { m_position.setZ(value); }
    inline float x() const { return m_position.x(); }
    inline float y() const { return m_position.y(); }
    inline float z() const { return m_position.z(); }

protected:
    void createExtraData();

    QScatterDataItemPrivate *d_ptr;

private:
    QVector3D m_position;
    QQuaternion m_rotation;
};

QT_END_NAMESPACE

#endif

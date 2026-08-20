// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QBARDATAITEM_H
#define QTDATAVIS3D_QBARDATAITEM_H

#include <QtDataVisualization/qdatavisualizationglobal.h>

QT_BEGIN_NAMESPACE

class QBarDataItemPrivate;

class Q_DATAVISUALIZATION_EXPORT QBarDataItem
{
public:
    QBarDataItem();
    QBarDataItem(float value);
    QBarDataItem(float value, float angle);
    QBarDataItem(const QBarDataItem &other);
    ~QBarDataItem();

    QBarDataItem &operator=(const QBarDataItem &other);

    inline void setValue(float val) { m_value = val; }
    inline float value() const { return m_value; }
    inline void setRotation(float angle) { m_angle = angle; }
    inline float rotation() const { return m_angle; }

protected:
    void createExtraData();

    QBarDataItemPrivate *d_ptr;

private:
    float m_value;
    float m_angle;
};

QT_END_NAMESPACE

#endif

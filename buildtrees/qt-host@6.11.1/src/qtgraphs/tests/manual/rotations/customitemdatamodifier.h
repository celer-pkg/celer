// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
//
#ifndef CUSTOMITEMDATAMODIFIER_H
#define CUSTOMITEMDATAMODIFIER_H

#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <QtGraphs/qscatterdataproxy.h>
#include <QtCore/QTimer>

class CustomItemDataModifier : public QObject
{
    Q_OBJECT
public:
    explicit CustomItemDataModifier(Q3DScatterWidgetItem *scatter);
    ~CustomItemDataModifier();


public Q_SLOTS:
    void toggleAbsolute(int absolute);
    void toggleTimer();
    void setXAxisReversed(int reveresed);
    void setYAxisReversed(int reveresed);
    void setZAxisReversed(int reveresed);
    void setCustomX(int value);
    void setCustomY(int value);
    void setCustomZ(int value);
    void setCustomRotation(int enabled);


private:
    void updatePositions();
    QQuaternion updateCustomRotation();
    Q3DScatterWidgetItem *m_graph;
    QTimer m_rotationTimer;
    QCustom3DItem *m_arrow;
    QCustom3DItem *m_sphere;
    QList<QCustom3DItem *> m_axisHelpers;
    QVector3D m_customEulerAngles = {0,0,0};
    bool m_customRotationEnabled = false;

    bool m_absoluteScaling = true;
    float m_radius = 7.0f;
};


#endif

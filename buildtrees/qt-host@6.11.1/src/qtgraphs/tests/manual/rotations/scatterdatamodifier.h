// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SCATTERDATAMODIFIER_H
#define SCATTERDATAMODIFIER_H

#include <QtGraphsWidgets/q3dscatterwidgetitem.h>
#include <QtGraphs/qscatterdataproxy.h>
#include <QtCore/QTimer>

class ScatterDataModifier : public QObject
{
    Q_OBJECT
public:
    explicit ScatterDataModifier(Q3DScatterWidgetItem *scatter);
    ~ScatterDataModifier();

    void generateData();

public Q_SLOTS:
    void setFieldLines(int lines);
    void setArrowsPerLine(int arrows);
    void toggleRotation();
    void triggerRotation();
    void toggleSun();

private:
    Q3DScatterWidgetItem *m_graph;
    QTimer m_rotationTimer;
    int m_fieldLines;
    int m_arrowsPerLine;
    QScatter3DSeries *m_magneticField;
    QCustom3DItem *m_sun;
    QScatterDataArray m_magneticFieldArray;
    float m_angleOffset;
    float m_angleStep;
};

#endif

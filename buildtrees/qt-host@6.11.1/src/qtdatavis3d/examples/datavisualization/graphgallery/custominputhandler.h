// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CUSTOMINPUTHANDLER_H
#define CUSTOMINPUTHANDLER_H

#include "highlightseries.h"

#include <QtDataVisualization/q3dinputhandler.h>
#include <QtDataVisualization/qabstract3dgraph.h>
#include <QtDataVisualization/qvalue3daxis.h>

class CustomInputHandler : public Q3DInputHandler
{
    Q_OBJECT

    enum InputState {
        StateNormal = 0,
        StateDraggingX,
        StateDraggingZ,
        StateDraggingY
    };

public:
    explicit CustomInputHandler(QAbstract3DGraph *graph, QObject *parent = 0);

    inline void setLimits(float min, float max, float minRange) {
        m_areaMinValue = min;
        m_areaMaxValue = max;
        m_axisXMinValue = m_areaMinValue;
        m_axisXMaxValue = m_areaMaxValue;
        m_axisZMinValue = m_areaMinValue;
        m_axisZMaxValue = m_areaMaxValue;
        m_axisXMinRange = minRange;
        m_axisZMinRange = minRange;
    }
    inline void setAxes(QValue3DAxis *axisX, QValue3DAxis *axisY, QValue3DAxis *axisZ) {
        m_axisX = axisX;
        m_axisY = axisY;
        m_axisZ = axisZ;
    }
    inline void setAspectRatio(float ratio) { m_aspectRatio = ratio; }
    inline void setHighlightSeries(HighlightSeries *series) { m_highlight = series; }
    inline void setDragSpeedModifier(float modifier) { m_speedModifier = modifier; }

protected:
    virtual void mousePressEvent(QMouseEvent *event, const QPoint &mousePos);
    virtual void mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos);
    virtual void mouseReleaseEvent(QMouseEvent *event, const QPoint &mousePos);
    virtual void wheelEvent(QWheelEvent *event);

private:
    void handleElementSelected(QAbstract3DGraph::ElementType type);
    void handleAxisDragging();
    void checkConstraints();

private:
    HighlightSeries *m_highlight = nullptr;
    bool m_mousePressed = false;
    InputState m_state = StateNormal;
    QValue3DAxis *m_axisX = nullptr;
    QValue3DAxis *m_axisY = nullptr;
    QValue3DAxis *m_axisZ = nullptr;
    float m_speedModifier = 20.f;
    float m_aspectRatio = 0.f;
    float m_axisXMinValue = 0.f;
    float m_axisXMaxValue = 0.f;
    float m_axisXMinRange = 0.f;
    float m_axisZMinValue = 0.f;
    float m_axisZMaxValue = 0.f;
    float m_axisZMinRange = 0.f;
    float m_areaMinValue = 0.f;
    float m_areaMaxValue = 0.f;
};

#endif

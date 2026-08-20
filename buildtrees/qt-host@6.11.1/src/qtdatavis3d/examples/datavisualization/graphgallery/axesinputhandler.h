// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef AXESINPUTHANDLER_H
#define AXESINPUTHANDLER_H

#include <QtDataVisualization/q3dinputhandler.h>
#include <QtDataVisualization/qabstract3dgraph.h>
#include <QtDataVisualization/qvalue3daxis.h>

//! [0]
class AxesInputHandler : public Q3DInputHandler
//! [0]
{
    Q_OBJECT

    enum InputState {
        StateNormal = 0,
        StateDraggingX,
        StateDraggingZ,
        StateDraggingY
    };

public:
    explicit AxesInputHandler(QAbstract3DGraph *graph, QObject *parent = 0);

    inline void setAxes(QValue3DAxis *axisX, QValue3DAxis *axisZ, QValue3DAxis *axisY) {
        m_axisX = axisX;
        m_axisZ = axisZ;
        m_axisY = axisY;
    }

    //! [1]
    inline void setDragSpeedModifier(float modifier) { m_speedModifier = modifier; }
    //! [1]

    virtual void mousePressEvent(QMouseEvent *event, const QPoint &mousePos);
    virtual void mouseMoveEvent(QMouseEvent *event, const QPoint &mousePos);
    virtual void mouseReleaseEvent(QMouseEvent *event, const QPoint &mousePos);

private:
    void handleElementSelected(QAbstract3DGraph::ElementType type);
    void handleAxisDragging();

private:
    bool m_mousePressed = false;
    InputState m_state = StateNormal;
    QValue3DAxis *m_axisX = nullptr;
    QValue3DAxis *m_axisZ = nullptr;
    QValue3DAxis *m_axisY = nullptr;
    float m_speedModifier = 15.f;
};

#endif

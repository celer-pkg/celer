// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Q3DCAMERA_P_H
#define Q3DCAMERA_P_H

#include "datavisualizationglobal_p.h"
#include "q3dcamera.h"
#include <QtGui/QMatrix4x4>

QT_BEGIN_NAMESPACE

class Q3DCamera;

class Q3DCameraPrivate : public QObject
{
    Q_OBJECT
public:
    Q3DCameraPrivate(Q3DCamera *q);
    ~Q3DCameraPrivate();

    void sync(Q3DCamera &other);

    void setXRotation(float rotation);
    void setYRotation(float rotation);
    void setMinXRotation(float rotation);
    float minXRotation() const;
    void setMinYRotation(float rotation);
    float minYRotation() const;
    void setMaxXRotation(float rotation);
    float maxXRotation() const;
    void setMaxYRotation(float rotation);
    float maxYRotation() const;

    void updateViewMatrix(float zoomAdjustment);

    QMatrix4x4 viewMatrix() const;
    void setViewMatrix(const QMatrix4x4 &viewMatrix);

    bool isViewMatrixAutoUpdateEnabled() const;
    void setViewMatrixAutoUpdateEnabled(bool isEnabled);

    void setBaseOrientation(const QVector3D &defaultPosition,
                            const QVector3D &defaultTarget,
                            const QVector3D &defaultUp);

    QVector3D calculatePositionRelativeToCamera(const QVector3D &relativePosition,
                                                float fixedRotation,
                                                float distanceModifier) const;

Q_SIGNALS:
    void minXRotationChanged(float rotation);
    void minYRotationChanged(float rotation);
    void maxXRotationChanged(float rotation);
    void maxYRotationChanged(float rotation);
    void viewMatrixChanged(const QMatrix4x4 &viewMatrix);
    void viewMatrixAutoUpdateChanged(bool enabled);

public:
    Q3DCamera *q_ptr;

    QVector3D m_actualTarget;
    QVector3D m_up;

    QMatrix4x4 m_viewMatrix;
    bool m_isViewMatrixUpdateActive;

    float m_xRotation;
    float m_yRotation;
    float m_minXRotation;
    float m_minYRotation;
    float m_maxXRotation;
    float m_maxYRotation;
    float m_zoomLevel;
    float m_minZoomLevel;
    float m_maxZoomLevel;
    bool m_wrapXRotation;
    bool m_wrapYRotation;
    Q3DCamera::CameraPreset m_activePreset;
    QVector3D m_requestedTarget;

    friend class Bars3DRenderer;
    friend class Surface3DRenderer;
    friend class Scatter3DRenderer;
    friend class SelectionPointer;
    friend class Q3DInputHandler;
    friend class QTouch3DInputHandler;
};

QT_END_NAMESPACE

#endif

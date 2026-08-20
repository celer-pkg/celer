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

#ifndef CAMERAHELPER_P_H
#define CAMERAHELPER_P_H

#include "datavisualizationglobal_p.h"
#include "q3dcamera.h"

QT_BEGIN_NAMESPACE
class QMatrix4x4;
class QPoint;
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE

class CameraHelper : public QObject
{
    Q_OBJECT

private:
    QVector3D m_position;
    QVector3D m_target;
    QVector3D m_up;

    QPoint m_previousMousePos;

    GLfloat m_xRotation;
    GLfloat m_yRotation;
    GLfloat m_defaultXRotation;
    GLfloat m_defaultYRotation;

    GLfloat m_rotationSpeed;

public:
    explicit CameraHelper(QObject *parent = 0);
    ~CameraHelper();

    // How fast camera rotates when mouse is dragged. Default is 100.
    void setRotationSpeed(int speed);
    // Set camera rotation in degrees
    void setCameraRotation(const QPointF &rotation);
    // Get camera rotations
    QPointF getCameraRotations();
    // Set default camera orientation. Position's x and y should be 0.
    void setDefaultCameraOrientation(const QVector3D &defaultPosition,
                                     const QVector3D &defaultTarget,
                                     const QVector3D &defaultUp);
    // Calculate view matrix based on rotation and zoom
    QMatrix4x4 calculateViewMatrix(const QPoint &mousePos, int zoom,
                                   int screenWidth, int screenHeight,
                                   bool showUnder = false);
    // Calcluate light position based on rotation. Call after calling calculateViewMatrix to get
    // up-to-date position
    QVector3D calculateLightPosition(const QVector3D &lightPosition,
                                     GLfloat fixedRotation = 0.0f,
                                     GLfloat distanceModifier = 0.0f);
    void updateMousePos(const QPoint &mousePos);
    void setCameraPreset(Q3DCamera::CameraPreset preset);
};

QT_END_NAMESPACE

#endif

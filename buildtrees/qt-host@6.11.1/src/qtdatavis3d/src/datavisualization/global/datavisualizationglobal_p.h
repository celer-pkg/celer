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

#ifndef DATAVISUALIZATIONGLOBAL_P_H
#define DATAVISUALIZATIONGLOBAL_P_H

#include "qdatavisualizationglobal.h"
#include <QtGui/QOpenGLFunctions>
#include <QtOpenGL/QOpenGLFramebufferObject>
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

// Constants used in several files
// Distance from camera to origin
static const GLfloat cameraDistance = 6.0f;
// Size of font to be used in label texture rendering. Doesn't affect the actual font size.
static const int textureFontSize = 50;
static const GLfloat defaultRatio = 1.0f / 1.6f; // default aspect ratio 16:10
#if !(defined QT_OPENGL_ES)
static const float gridLineOffset = 0.0001f; // Offset for lifting grid lines off background
#else
static const float gridLineOffset = 0.0035f; // Offset for lifting grid lines off background
#endif
// Default light position. To have shadows working correctly, light should be as far as camera, or a bit further
// y position is added to the minimum height (or can be thought to be that much above or below the camera)
static const QVector3D defaultLightPos = QVector3D(0.0f, 0.5f, 0.0f);
static const QVector3D zeroVector = QVector3D(0.0f, 0.0f, 0.0f);
static const QVector3D oneVector = QVector3D(1.0f, 1.0f, 1.0f);
static const QVector3D upVector = QVector3D(0.0f, 1.0f, 0.0f);
static const QVector3D cameraDistanceVector = QVector3D(0.0f, 0.0f, cameraDistance);
static const QQuaternion identityQuaternion;

// Skip color == selection texture's background color
static const QVector4D selectionSkipColor = QVector4D(255.0f, 255.0f, 255.0f, 255.0f);
static const QVector4D invalidColorVector = QVector4D(-1.0f, -1.0f, -1.0f, -1.0f);
static const GLfloat itemAlpha = 0.0f;
static const GLfloat customItemAlpha = 252.0f;
static const GLfloat labelValueAlpha = 253.0f;
static const GLfloat labelRowAlpha = 254.0f;
static const GLfloat labelColumnAlpha = 255.0f;
static const GLfloat gradientTextureHeight = 1024.0f;
static const GLfloat gradientTextureWidth = 2.0f;
static const GLfloat uniformTextureHeight = 64.0f;
static const GLfloat uniformTextureWidth = 2.0f;
static const GLfloat labelMargin = 0.05f;
static const GLfloat gridLineWidth = 0.005f;

QT_END_NAMESPACE

#endif

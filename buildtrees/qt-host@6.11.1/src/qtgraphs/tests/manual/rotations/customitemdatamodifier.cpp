// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
//
#include "customitemdatamodifier.h"
#include "scatterdatamodifier.h"
#include <QtGraphs/qscatterdataproxy.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qscatter3dseries.h>
#include <QtGraphs/QCustom3DItem>

#include <QtGui/qquaternion.h>

#include <QtCore/qmath.h>
#include <algorithm>
#include <cstdlib>

CustomItemDataModifier::CustomItemDataModifier(Q3DScatterWidgetItem *scatter)
    : m_graph(scatter),
      m_arrow(new QCustom3DItem),
      m_sphere(new QCustom3DItem),
      m_radius(7.0f)
{

    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::None);
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::Front);

    m_sphere->setParent(m_graph);
    m_sphere->setMeshFile(QStringLiteral(":/mesh/largesphere.mesh"));
    QImage sphereTex = QImage(2, 2, QImage::Format_RGB32);
    sphereTex.fill(QColor(0xff, 0xbb, 0x00));
    m_sphere->setTextureImage(sphereTex);
    m_sphere->setScaling(QVector3D(0.01f, 0.01f, 0.01f));

    m_arrow->setParent(m_graph);
    m_arrow->setMeshFile(QStringLiteral(":/mesh/narrowarrow.mesh"));
    QImage arrowTex = QImage(2, 2, QImage::Format_RGB32);
    arrowTex.fill(QColor(0x00, 0xbb, 0xff));
    m_arrow->setTextureImage(arrowTex);

    m_arrow->setScaling(QVector3D(0.1f, 0.1f, 0.1f));
    m_arrow->setPosition({0,0,0});

    m_graph->addCustomItem(m_arrow);
    m_graph->addCustomItem(m_sphere);

    m_graph->axisX()->setRange(-10, 10);
    m_graph->axisX()->setTitle("X");
    m_graph->axisX()->setTitleVisible(true);
    m_graph->axisY()->setRange(-10, 10);
    m_graph->axisY()->setTitle("Y");
    m_graph->axisY()->setTitleVisible(true);
    m_graph->axisZ()->setRange(-10, 10);
    m_graph->axisZ()->setTitle("Z");
    m_graph->axisZ()->setTitleVisible(true);

    // m_graph->setAspectRatio(1.0);
    m_axisHelpers.reserve(3);
    QCustom3DItem *xAxisHelper = new QCustom3DItem();
    xAxisHelper->setParent(m_graph);
    xAxisHelper->setMeshFile(QStringLiteral(":/mesh/narrowarrow.mesh"));
    xAxisHelper->setScaling({0.1f, 0.1f, 0.1f});
    xAxisHelper->setPosition({2.2f, -1.0f, 2.0f});
    xAxisHelper->setPositionAbsolute(true);
    xAxisHelper->setRotation({0.707f, .0f, 0.f , -0.707f});
    QImage xTex = QImage(2, 2, QImage::Format_RGB32);
    xTex.fill(QColor(0xff, 0x00, 0x00));
    xAxisHelper->setTextureImage(xTex);
    m_graph->addCustomItem(xAxisHelper);
    m_axisHelpers.append(xAxisHelper);

    m_axisHelpers.reserve(3);
    QCustom3DItem *yAxisHelper = new QCustom3DItem();
    yAxisHelper->setParent(m_graph);
    yAxisHelper->setMeshFile(QStringLiteral(":/mesh/narrowarrow.mesh"));
    yAxisHelper->setScaling({0.1f, 0.1f, 0.1f});
    yAxisHelper->setPosition({2.0f, -0.8f, 2.0f});
    yAxisHelper->setPositionAbsolute(true);
    QImage yTex = QImage(2, 2, QImage::Format_RGB32);
    yTex.fill(QColor(0x00, 0xff, 0x00));
    yAxisHelper->setTextureImage(yTex);
    m_graph->addCustomItem(yAxisHelper);
    m_axisHelpers.append(yAxisHelper);

    QCustom3DItem *zAxisHelper = new QCustom3DItem();
    zAxisHelper->setParent(m_graph);
    zAxisHelper->setMeshFile(QStringLiteral(":/mesh/narrowarrow.mesh"));
    zAxisHelper->setScaling({0.1f, 0.1f, 0.1f});
    zAxisHelper->setPosition({2.0f, -1.0f, 2.2f});
    //note: z axis is reversed by default in qt graphs
    zAxisHelper->setRotation({0.707f, 0.707f, .0f, .0f});
    zAxisHelper->setPositionAbsolute(true);
    QImage zTex = QImage(2, 2, QImage::Format_RGB32);
    zTex.fill(QColor(0x00, 0x00, 0xff));
    zAxisHelper->setTextureImage(zTex);
    m_graph->addCustomItem(zAxisHelper);
    m_axisHelpers.append(zAxisHelper);
    //create axis helper


    m_absoluteScaling = m_sphere->isScalingAbsolute();
    m_arrow->setRotationAbsolute(m_absoluteScaling);
    toggleAbsolute(0);
    updatePositions();
    toggleTimer();
    QObject::connect(&m_rotationTimer, &QTimer::timeout, this,
                     &CustomItemDataModifier::updatePositions);
}

CustomItemDataModifier::~CustomItemDataModifier()
{
    delete m_graph;
}

void CustomItemDataModifier::toggleAbsolute(int absolute)
{
    m_absoluteScaling = bool(absolute);
    qDebug() << absolute;

    m_sphere->setScalingAbsolute(m_absoluteScaling);
    m_arrow->setScalingAbsolute(m_absoluteScaling);
    m_sphere->setPositionAbsolute(m_absoluteScaling);
    m_arrow->setPositionAbsolute(m_absoluteScaling);
    m_arrow->setRotationAbsolute(m_absoluteScaling);

    if (m_absoluteScaling) {
        m_sphere->setScaling(QVector3D(0.01f, 0.01f, 0.01f));
        m_arrow->setScaling(QVector3D(0.1f, 0.1f, 0.1f));
        m_radius = 1.0;
    } else {
        m_sphere->setScaling(QVector3D(0.1f, 0.1f, 0.1f));
        m_arrow->setScaling(QVector3D(2.0f, 2.0f, 2.0f));
        m_radius = 7.0;
    }
}

void CustomItemDataModifier::toggleTimer()
{
    if (m_rotationTimer.isActive())
        m_rotationTimer.stop();
    else
        m_rotationTimer.start(16.7);
}

void CustomItemDataModifier::setXAxisReversed(int reversed)
{
    m_graph->axisX()->setReversed(bool(reversed));
    if (!m_rotationTimer.isActive())
        updatePositions();
}

void CustomItemDataModifier::setYAxisReversed(int reversed)
{
    m_graph->axisY()->setReversed(bool(reversed));
    if (!m_rotationTimer.isActive())
        updatePositions();
}

void CustomItemDataModifier::setZAxisReversed(int reversed)
{
    m_graph->axisZ()->setReversed(bool(reversed));
    if (!m_rotationTimer.isActive())
        updatePositions();
}

void CustomItemDataModifier::setCustomX(int value)
{
    m_customEulerAngles.setX(float(value));
    updatePositions();
}

void CustomItemDataModifier::setCustomY(int value)
{
    m_customEulerAngles.setY(float(value));
    updatePositions();
}

void CustomItemDataModifier::setCustomZ(int value)
{
    m_customEulerAngles.setZ(float(value));
    updatePositions();
}

void CustomItemDataModifier::setCustomRotation(int enabled)
{
    m_customRotationEnabled = bool(enabled);
    m_sphere->setVisible(!m_customRotationEnabled);
    m_rotationTimer.stop();
}

void CustomItemDataModifier::updatePositions()
{
    static int increment = 0;
    constexpr float n = 3;
    const float maxFps = 60 * n;

    float time = float(increment) * M_PI * 2 / maxFps;

    const float r = m_radius;
    const float x =  r * cos(time) * cos(n * time);
    const float y =  r * sin(time) * cos(n * time);
    const float z =  r * sin(n * time);
    QVector3D spherePos(x,y,z);
    m_sphere->setPosition(spherePos);

    if (m_customRotationEnabled) {
        m_arrow->setRotation(QQuaternion::fromEulerAngles(m_customEulerAngles));
        return;
    }
    //look at algorithm
    QVector3D forwardDirection(0, 1, 0);
    QVector3D upDirection(0, 1, 0);
    if (m_graph->axisY()->reversed() && !m_arrow->isRotationAbsolute()) {
        upDirection = QVector3D(0, -1, 0);
        forwardDirection = QVector3D(0, -1, 0);
    }

    QVector3D targetDirection = spherePos.normalized();
    QVector3D rotationAxis = QVector3D::crossProduct(forwardDirection, targetDirection);
    QVector3D normalizedAxis = rotationAxis.normalized();

    if (qFuzzyIsNull(normalizedAxis.lengthSquared()))
        rotationAxis = upDirection;

    float dot = QVector3D::dotProduct(forwardDirection, targetDirection);
    float rotationAngle = qRadiansToDegrees(qAcos(dot));

    QQuaternion lookAtRot = QQuaternion::fromAxisAndAngle(rotationAxis, rotationAngle);
    m_arrow->setRotation(lookAtRot);

    increment = (increment + 1) % int(maxFps);
}

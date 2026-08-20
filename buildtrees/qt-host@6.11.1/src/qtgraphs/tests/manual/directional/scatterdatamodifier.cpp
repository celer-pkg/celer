// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterdatamodifier.h"
#include <QtGraphs/qscatterdataproxy.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qscatter3dseries.h>
#include <qmath.h>
#include <QComboBox>

#include <QtGui/qquaternion.h>

const int numberOfCols = 8;
const int numberOfRows = 8;
const float limit = 8.0f;
#define HEDGEHOG

ScatterDataModifier::ScatterDataModifier(Q3DScatterWidgetItem *scatter)
    : m_graph(scatter),
      m_fontSize(40.0f),
      m_style(QAbstract3DSeries::Mesh::UserDefined),
      m_smooth(true)
{
    m_graph->activeTheme()->setTheme(QGraphsTheme::Theme::QtGreen);
    QFont font = m_graph->activeTheme()->labelFont();
    font.setPointSize(m_fontSize);
    m_graph->activeTheme()->setLabelFont(font);
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::SoftLow);
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::Front);

    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    QScatterDataProxy *proxy = new QScatterDataProxy;
    QScatter3DSeries *series = new QScatter3DSeries(proxy);
    series->setItemLabelFormat("@xTitle: @xLabel @yTitle: @yLabel @zTitle: @zLabel");
    m_graph->addSeries(series);

    QObject::connect(&m_rotationTimer, &QTimer::timeout, this,
                     &ScatterDataModifier::triggerRotation);

    addData();
}

ScatterDataModifier::~ScatterDataModifier()
{
    delete m_graph;
}

void ScatterDataModifier::fpsChanged(int fps)
{
    qDebug() << "Current FPS:" << fps;
}

void ScatterDataModifier::addData()
{
    // Configure the axes according to the data
    m_graph->axisX()->setTitle("X");
    m_graph->axisY()->setTitle("Y");
    m_graph->axisZ()->setTitle("Z");
    m_graph->axisX()->setRange(-limit, limit);
    m_graph->axisY()->setRange(-1.0f, 1.0f);
    m_graph->axisZ()->setRange(-limit, limit);

    QScatterDataArray dataArray(numberOfCols * numberOfRows);

    float angleStep = 360.0f / float(numberOfCols);
    float latAngleStep = 100.0f / float(numberOfRows);

    int arrayIndex = 0;
    for (float i = 0; i < numberOfRows; i++) {
        float latAngle = float(i) * latAngleStep + 40.0f;
        float radius = qSin(qDegreesToRadians(latAngle)) * limit;
        float y = qCos(qDegreesToRadians(latAngle)) * 1.0f;
#ifdef HEDGEHOG
        float angleZ = qRadiansToDegrees(qAtan((y * limit / 2.0f) / radius));
        QQuaternion rotationZ = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), angleZ - 90.0f);
#endif
        for (float j = 0; j < numberOfCols; j++) {
            float angle = float(j) * angleStep;
            float x = qCos(qDegreesToRadians(angle)) * radius;
            float z = qSin(qDegreesToRadians(angle)) * radius;

            float angleY = qRadiansToDegrees(qAtan(z / x));
            if (x < 0)
                angleY = 180.0f + angleY;
            if (x > 0 && z < 0)
                angleY = 360.0f + angleY;
#ifdef HEDGEHOG
            QQuaternion rotationY = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), angleY);
            QQuaternion rotation = rotationY * rotationZ;
#else
            QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), angleY) *
                QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), -90.0f);
#endif

            dataArray[arrayIndex].setPosition(QVector3D(x, y, z));
            dataArray[arrayIndex].setRotation(rotation);
            ++arrayIndex;
        }
    }

    m_graph->seriesList().at(0)->dataProxy()->resetArray(dataArray);
}

void ScatterDataModifier::enableOptimization(int enabled)
{
    if (enabled)
        m_graph->setOptimizationHint(QtGraphs3D::OptimizationHint::Default);
    else
        m_graph->setOptimizationHint(QtGraphs3D::OptimizationHint::Legacy);
}

void ScatterDataModifier::changeStyle(int style)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        m_style = QAbstract3DSeries::Mesh(comboBox->itemData(style).toInt());
        if (m_graph->seriesList().size())
            m_graph->seriesList().at(0)->setMesh(m_style);
    }
}

void ScatterDataModifier::changeTheme(int theme)
{
    QGraphsTheme *currentTheme = m_graph->activeTheme();
    currentTheme->setTheme(QGraphsTheme::Theme(theme));
    emit backgroundVisibleChanged(currentTheme->isPlotAreaBackgroundVisible());
    emit gridVisibleChanged(currentTheme->isGridVisible());
    emit fontChanged(currentTheme->labelFont());
}

void ScatterDataModifier::changePresetCamera()
{
    static int preset = int(QtGraphs3D::CameraPreset::FrontLow);

    m_graph->setCameraPreset((QtGraphs3D::CameraPreset) preset);

    if (++preset > int(QtGraphs3D::CameraPreset::DirectlyBelow))
        preset = int(QtGraphs3D::CameraPreset::FrontLow);
}

void ScatterDataModifier::changeLabelStyle()
{
    m_graph->activeTheme()->setLabelBackgroundVisible(
        !m_graph->activeTheme()->isLabelBackgroundVisible());
}

void ScatterDataModifier::changeFont(const QFont &font)
{
    QFont newFont = font;
    newFont.setPointSizeF(m_fontSize);
    m_graph->activeTheme()->setLabelFont(newFont);
}

void ScatterDataModifier::shadowQualityUpdatedByVisual(QtGraphs3D::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality); // connected to a checkbox in main.cpp
}

void ScatterDataModifier::triggerRotation()
{
    if (m_graph->seriesList().size()) {
        static float seriesAngle = 0.0f;
        QQuaternion rotation = QQuaternion::fromAxisAndAngle(1.0f, 1.0f, 1.0f, seriesAngle++);
        m_graph->seriesList().at(0)->setMeshRotation(rotation);
    }
}

void ScatterDataModifier::changeShadowQuality(int quality)
{
    QtGraphs3D::ShadowQuality sq = QtGraphs3D::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}

void ScatterDataModifier::setBackgroundVisible(int visible)
{
    m_graph->activeTheme()->setPlotAreaBackgroundVisible((bool)visible);
}

void ScatterDataModifier::setGridVisible(int visible)
{
    m_graph->activeTheme()->setGridVisible((bool)visible);
}

void ScatterDataModifier::toggleRotation()
{
    if (m_rotationTimer.isActive())
        m_rotationTimer.stop();
    else
        m_rotationTimer.start(20);
}

// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "scatterdatamodifier.h"
#include <QtGraphs/qscatterdataproxy.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphs/q3dscene.h>
#include <QtGraphs/qscatter3dseries.h>
#include <QtGraphs/QCustom3DItem>

#include <QtGui/qquaternion.h>

#include <QtCore/qmath.h>

static const float verticalRange = 8.0f;
static const float horizontalRange = verticalRange;
static const float ellipse_a = horizontalRange / 3.0f;
static const float ellipse_b = verticalRange;
static const float doublePi = float(M_PI) * 2.0f;
static const float radiansToDegrees = 360.0f / doublePi;
static const float animationFrames = 30.0f;

ScatterDataModifier::ScatterDataModifier(Q3DScatterWidgetItem *scatter)
    : m_graph(scatter),
      m_fieldLines(12),
      m_arrowsPerLine(16),
      m_magneticField(new QScatter3DSeries),
      m_sun(new QCustom3DItem),
      m_angleOffset(0.0f),
      m_angleStep(doublePi / m_arrowsPerLine / animationFrames)
{
    m_graph->setShadowQuality(QtGraphs3D::ShadowQuality::None);
    m_graph->setCameraPreset(QtGraphs3D::CameraPreset::Front);

    // Magnetic field lines use custom narrow arrow
    m_magneticField->setItemSize(0.2f);
    //! [3]
    m_magneticField->setMesh(QAbstract3DSeries::Mesh::UserDefined);
    m_magneticField->setUserDefinedMesh(QStringLiteral(":/mesh/narrowarrow.mesh"));
    //! [3]
    //! [4]
    QLinearGradient fieldGradient(0, 0, 16, 1024);
    fieldGradient.setColorAt(0.0, Qt::black);
    fieldGradient.setColorAt(1.0, Qt::white);
    m_magneticField->setBaseGradient(fieldGradient);
    m_magneticField->setColorStyle(QGraphsTheme::ColorStyle::RangeGradient);
    //! [4]

    // For 'sun' we use a custom large sphere
    m_sun->setScaling(QVector3D(0.07f, 0.07f, 0.07f));
    m_sun->setMeshFile(QStringLiteral(":/mesh/largesphere.mesh"));
    QImage sunColor = QImage(2, 2, QImage::Format_RGB32);
    sunColor.fill(QColor(0xff, 0xbb, 0x00));
    m_sun->setTextureImage(sunColor);

    m_graph->addSeries(m_magneticField);
    m_graph->addCustomItem(m_sun);

    // Configure the axes according to the data
    m_graph->axisX()->setRange(-horizontalRange, horizontalRange);
    m_graph->axisY()->setRange(-verticalRange, verticalRange);
    m_graph->axisZ()->setRange(-horizontalRange, horizontalRange);
    m_graph->axisX()->setSegmentCount(int(horizontalRange));
    m_graph->axisZ()->setSegmentCount(int(horizontalRange));

    QObject::connect(&m_rotationTimer, &QTimer::timeout, this,
                     &ScatterDataModifier::triggerRotation);

    toggleRotation();
    generateData();
}

ScatterDataModifier::~ScatterDataModifier()
{
    delete m_graph;
}

void ScatterDataModifier::generateData()
{
    int arraySize = m_fieldLines * m_arrowsPerLine;
    if (arraySize != m_magneticFieldArray.size())
        m_magneticFieldArray.resize(arraySize);

    QScatterDataItem *ptrToDataArray = &m_magneticFieldArray.first();

    for (float i = 0; i < m_fieldLines; i++) {
        float horizontalAngle = (doublePi * i) / m_fieldLines;
        float xCenter = ellipse_a * qCos(horizontalAngle);
        float zCenter = ellipse_a * qSin(horizontalAngle);

        // Rotate - arrow always tangential to origin
        //! [0]
        QQuaternion yRotation = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, horizontalAngle * radiansToDegrees);
        //! [0]

        for (float j = 0; j < m_arrowsPerLine; j++) {
            // Calculate point on ellipse centered on origin and parallel to x-axis
            float verticalAngle = ((doublePi * j) / m_arrowsPerLine) + m_angleOffset;
            float xUnrotated = ellipse_a * qCos(verticalAngle);
            float y = ellipse_b * qSin(verticalAngle);

            // Rotate the ellipse around y-axis
            float xRotated = xUnrotated * qCos(horizontalAngle);
            float zRotated = xUnrotated * qSin(horizontalAngle);

            // Add offset
            float x = xCenter + xRotated;
            float z = zCenter + zRotated;

            //! [1]
            QQuaternion zRotation = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, verticalAngle * radiansToDegrees);
            QQuaternion totalRotation = yRotation * zRotation;
            //! [1]

            ptrToDataArray->setPosition(QVector3D(x, y, z));
            //! [2]
            ptrToDataArray->setRotation(totalRotation);
            //! [2]
            ptrToDataArray++;
        }
    }

    if (m_graph->selectedSeries() == m_magneticField)
        m_graph->clearSelection();

    m_magneticField->dataProxy()->resetArray(m_magneticFieldArray);
}

void ScatterDataModifier::setFieldLines(int lines)
{
    m_fieldLines = lines;
    generateData();
}

void ScatterDataModifier::setArrowsPerLine(int arrows)
{
    m_angleOffset = 0.0f;
    m_angleStep = doublePi / m_arrowsPerLine / animationFrames;
    m_arrowsPerLine = arrows;
    generateData();
}

void ScatterDataModifier::triggerRotation()
{
    m_angleOffset += m_angleStep;
    generateData();
}

void ScatterDataModifier::toggleSun()
{
    m_sun->setVisible(!m_sun->isVisible());
}

void ScatterDataModifier::toggleRotation()
{
    if (m_rotationTimer.isActive())
        m_rotationTimer.stop();
    else
        m_rotationTimer.start(15);
}

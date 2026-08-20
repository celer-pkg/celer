// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "gridgeometry_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendergeometry_p.h>

/*!
    \qmltype GridGeometry
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \brief A custom geometry provider for rendering grids.

    This helper implements grid geometry, which allows showing a grid in a scene.

    For example, the following snippet would display a grid with 19 cells in
    both directions in a scene that has one light. Without further
    transformations, the grid is facing the camera by default.

    \image gridgeometry.jpg
           {19 by 19 cell grid facing camera with directional lighting}

    \badcode
        View3D {
            anchors.fill: parent
            camera: camera

            PerspectiveCamera {
                id: camera
                position: Qt.vector3d(0, 0, 600)
            }

            DirectionalLight {
                position: Qt.vector3d(-500, 500, -100)
                color: Qt.rgba(0.4, 0.2, 0.6, 1.0)
                ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
            }

            Model {
                scale: Qt.vector3d(100, 100, 100)
                geometry: GridGeometry {
                    horizontalLines: 20
                    verticalLines: 20
                }
                materials: [ DefaultMaterial { } ]
            }
        }
    \endcode

    \sa {Qt Quick 3D - Custom Geometry Example}, Model
*/

/*! \qmlproperty int GridGeometry::horizontalLines
    Specifies the number of horizontal lines in a grid. The default value is 1000.
*/
/*! \qmlproperty int GridGeometry::verticalLines
    Specifies the number of vertical lines in a grid. The default value is 1000.
*/
/*! \qmlproperty real GridGeometry::horizontalStep
    Specifies the spacing between horizontal lines. The default value is 0.1.
*/
/*! \qmlproperty real GridGeometry::verticalStep
    Specifies the spacing between vertical lines. The default value is 0.1.
*/

GridGeometry::GridGeometry()
    : QQuick3DGeometry()
{
    updateData();
}

GridGeometry::~GridGeometry()
{

}

int GridGeometry::horizontalLines() const
{
    return m_horLines;
}

int GridGeometry::verticalLines() const
{
    return m_vertLines;
}

float GridGeometry::horizontalStep() const
{
    return m_horStep;
}

float GridGeometry::verticalStep() const
{
    return m_vertStep;
}

void GridGeometry::setHorizontalLines(int count)
{
    count = qMax(count, 1);
    if (m_horLines == count)
        return;
    m_horLines = qMax(count, 1);
    emit horizontalLinesChanged();
    updateData();
    update();
    emit geometryChanged();
}

void GridGeometry::setVerticalLines(int count)
{
    count = qMax(count, 1);
    if (m_vertLines == count)
        return;
    m_vertLines = qMax(count, 1);
    emit verticalLinesChanged();
    updateData();
    update();
    emit geometryChanged();
}

void GridGeometry::setHorizontalStep(float step)
{
    step = qMax(step, 0.0f);
    if (qFuzzyCompare(m_horStep, step))
        return;
    m_horStep = step;
    emit horizontalStepChanged();
    updateData();
    update();
    emit geometryChanged();
}

void GridGeometry::setVerticalStep(float step)
{
    step = qMax(step, 0.0f);
    if (qFuzzyCompare(m_vertStep, step))
        return;
    m_vertStep = step;
    emit verticalStepChanged();
    updateData();
    update();
    emit geometryChanged();
}

static void fillVertexData(QByteArray &vertexData, int horLines, float horStep,
                           int vertLines, float vertStep)
{
    const int size = (horLines + vertLines) * sizeof(float) * 8 * 2;
    vertexData.resize(size);
    float *dataPtr = reinterpret_cast<float *>(vertexData.data());

    float y0 = -float(horLines - 1) * horStep * .5f;
    float x0 = -float(vertLines - 1) * vertStep * .5f;
    float y1 = -y0;
    float x1 = -x0;

    for (int i = 0; i < horLines; ++i) {
        // start position
        dataPtr[0] = x0;
        dataPtr[1] = y0 + i * horStep;
        dataPtr[2] = .0f;
        dataPtr[3] = 1.f;

        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.f;
        dataPtr[7] = 0;

        dataPtr += 8;

        // end position
        dataPtr[0] = x1;
        dataPtr[1] = y0 + i * horStep;
        dataPtr[2] = .0f;
        dataPtr[3] = 1.f;

        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.f;
        dataPtr[7] = 0;

        dataPtr += 8;
    }

    for (int i = 0; i < vertLines; ++i) {
        // start position
        dataPtr[0] = x0 + i * vertStep;
        dataPtr[1] = y0;
        dataPtr[2] = .0f;
        dataPtr[3] = 1.f;

        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.f;
        dataPtr[7] = 0;

        dataPtr += 8;

        // end position
        dataPtr[0] = x0 + i * vertStep;
        dataPtr[1] = y1;
        dataPtr[2] = .0f;
        dataPtr[3] = 1.f;

        dataPtr[4] = 0;
        dataPtr[5] = 0;
        dataPtr[6] = 1.f;
        dataPtr[7] = 0;

        dataPtr += 8;
    }
}

void GridGeometry::updateData()
{
    QByteArray vertexData;
    fillVertexData(vertexData, m_horLines, m_horStep, m_vertLines, m_vertStep);
    clear();
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, 16,
                 QQuick3DGeometry::Attribute::ComponentType::F32Type);
    setStride(32);
    setVertexData(vertexData);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Lines);
    setBounds(QVector3D(float(-m_vertLines / 2), float(-m_horLines / 2), 0.0f) * m_horStep,
              QVector3D(float(m_vertLines / 2), float(m_horLines / 2), 0.0f) * m_vertStep);
}

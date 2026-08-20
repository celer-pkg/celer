// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "axisgrid_p.h"

QT_BEGIN_NAMESPACE

AxisGrid::AxisGrid(QQuickItem *parent) :
      QQuickShaderEffect(parent)
{
}

AxisGrid::~AxisGrid() {}

void AxisGrid::componentComplete()
{
    QQuickShaderEffect::componentComplete();
    setupShaders();
}

void AxisGrid::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    m_iResolution = QVector3D(newGeometry.width(), newGeometry.height(), 1.0);
    emit iResolutionChanged();

    QQuickShaderEffect::geometryChange(newGeometry, oldGeometry);
}

void AxisGrid::setupShaders()
{
    setFragmentShader(QUrl(QStringLiteral("qrc:/shaders/gridshader.frag.qsb")));
    setVertexShader(QUrl(QStringLiteral("qrc:/shaders/gridshader.vert.qsb")));
}

QVector3D AxisGrid::iResolution() const
{
    return m_iResolution;
}

qreal AxisGrid::smoothing() const
{
    return m_smoothing;
}

void AxisGrid::setSmoothing(qreal newSmoothing)
{
    if (QtPrivate::fuzzyCompare(m_smoothing, newSmoothing))
        return;
    m_smoothing = newSmoothing;
    emit smoothingChanged();
}

int AxisGrid::origo() const
{
    return m_origo;
}

void AxisGrid::setOrigo(int newOrigo)
{
    if (m_origo == newOrigo)
        return;
    m_origo = newOrigo;
    emit origoChanged();
}

QVector4D AxisGrid::gridVisibility() const
{
    return m_gridVisibility;
}

void AxisGrid::setGridVisibility(const QVector4D &newGridVisibility)
{
    if (m_gridVisibility == newGridVisibility)
        return;
    m_gridVisibility = newGridVisibility;
    emit gridVisibilityChanged();
}

qreal AxisGrid::gridWidth() const
{
    return m_gridWidth;
}

void AxisGrid::setGridWidth(qreal newGridWidth)
{
    if (QtPrivate::fuzzyCompare(m_gridWidth, newGridWidth))
        return;
    m_gridWidth = newGridWidth;
    emit gridWidthChanged();
}

qreal AxisGrid::gridHeight() const
{
    return m_gridHeight;
}

void AxisGrid::setGridHeight(qreal newGridHeight)
{
    if (QtPrivate::fuzzyCompare(m_gridHeight, newGridHeight))
        return;
    m_gridHeight = newGridHeight;
    emit gridHeightChanged();
}

QPointF AxisGrid::gridMovement() const
{
    return m_gridMovement;
}

void AxisGrid::setGridMovement(QPointF newGridMovement)
{
    if (m_gridMovement == newGridMovement)
        return;
    m_gridMovement = newGridMovement;
    emit gridMovementChanged();
}

QColor AxisGrid::subGridColor() const
{
    return m_subGridColor;
}

void AxisGrid::setSubGridColor(QColor newSubGridColor)
{
    if (m_subGridColor == newSubGridColor)
        return;
    m_subGridColor = newSubGridColor;
    emit subGridColorChanged();
}

QColor AxisGrid::gridColor() const
{
    return m_gridColor;
}

void AxisGrid::setGridColor(QColor newGridColor)
{
    if (m_gridColor == newGridColor)
        return;
    m_gridColor = newGridColor;
    emit gridColorChanged();
}

QColor AxisGrid::plotAreaBackgroundColor() const
{
    return m_plotAreaBackgroundColor;
}

void AxisGrid::setPlotAreaBackgroundColor(QColor color)
{
    if (m_plotAreaBackgroundColor == color)
        return;
    m_plotAreaBackgroundColor = color;
    emit plotAreaBackgroundColorChanged();
}

qreal AxisGrid::subGridLineWidth() const
{
    return m_subGridLineWidth;
}

void AxisGrid::setSubGridLineWidth(qreal newSubGridLineWidth)
{
    if (QtPrivate::fuzzyCompare(m_subGridLineWidth, newSubGridLineWidth))
        return;
    m_subGridLineWidth = newSubGridLineWidth;
    emit subGridLineWidthChanged();
}

qreal AxisGrid::gridLineWidth() const
{
    return m_gridLineWidth;
}

void AxisGrid::setGridLineWidth(qreal newGridLineWidth)
{
    if (QtPrivate::fuzzyCompare(m_gridLineWidth, newGridLineWidth))
        return;
    m_gridLineWidth = newGridLineWidth;
    emit gridLineWidthChanged();
}

qreal AxisGrid::verticalSubGridScale() const
{
    return m_verticalSubGridScale;
}

void AxisGrid::setVerticalSubGridScale(qreal newVerticalSubGridScale)
{
    if (QtPrivate::fuzzyCompare(m_verticalSubGridScale, newVerticalSubGridScale))
        return;
    m_verticalSubGridScale = newVerticalSubGridScale;
    emit verticalSubGridScaleChanged();
}

qreal AxisGrid::horizontalSubGridScale() const
{
    return m_horizontalSubGridScale;
}

void AxisGrid::setHorizontalSubGridScale(qreal newHorizontalSubGridScale)
{
    if (QtPrivate::fuzzyCompare(m_horizontalSubGridScale, newHorizontalSubGridScale))
        return;
    m_horizontalSubGridScale = newHorizontalSubGridScale;
    emit horizontalSubGridScaleChanged();
}

QT_END_NAMESPACE

#include "moc_axisgrid_p.cpp"

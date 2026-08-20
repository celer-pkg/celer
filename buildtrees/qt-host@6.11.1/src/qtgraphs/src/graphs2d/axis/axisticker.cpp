// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "axisticker_p.h"

QT_BEGIN_NAMESPACE

AxisTicker::AxisTicker(QQuickItem *parent) :
      QQuickShaderEffect(parent)
{
}

AxisTicker::~AxisTicker() {}

void AxisTicker::componentComplete()
{
    QQuickShaderEffect::componentComplete();
    setupShaders();
}

void AxisTicker::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    m_iResolution = QVector3D(newGeometry.width(), newGeometry.height(), 1.0);
    emit iResolutionChanged();

    QQuickShaderEffect::geometryChange(newGeometry, oldGeometry);
}

void AxisTicker::setupShaders()
{
    if (m_isHorizontal) {
        setFragmentShader(QUrl(QStringLiteral("qrc:/shaders/tickershaderhorizontal.frag.qsb")));
        setVertexShader(QUrl(QStringLiteral("qrc:/shaders/tickershaderhorizontal.vert.qsb")));
    } else {
        setFragmentShader(QUrl(QStringLiteral("qrc:/shaders/tickershader.frag.qsb")));
        setVertexShader(QUrl(QStringLiteral("qrc:/shaders/tickershader.vert.qsb")));
    }
}

QVector3D AxisTicker::iResolution() const
{
    return m_iResolution;
}

qreal AxisTicker::smoothing() const
{
    return m_smoothing;
}

void AxisTicker::setSmoothing(qreal newSmoothing)
{
    if (QtPrivate::fuzzyCompare(m_smoothing, newSmoothing))
        return;
    m_smoothing = newSmoothing;
    emit smoothingChanged();
}

int AxisTicker::origo() const
{
    return m_origo;
}

void AxisTicker::setOrigo(int newOrigo)
{
    if (m_origo == newOrigo)
        return;
    m_origo = newOrigo;
    emit origoChanged();
}

bool AxisTicker::subTicksVisible() const
{
    return m_subTicksVisible;
}

void AxisTicker::setSubTicksVisible(bool newSubTicksVisible)
{
    if (m_subTicksVisible == newSubTicksVisible)
        return;
    m_subTicksVisible = newSubTicksVisible;
    emit subTicksVisibleChanged();
}

qreal AxisTicker::spacing() const
{
    return m_spacing;
}

void AxisTicker::setSpacing(qreal newSpacing)
{
    if (QtPrivate::fuzzyCompare(m_spacing, newSpacing))
        return;
    m_spacing = newSpacing;
    emit spacingChanged();
}

qreal AxisTicker::displacement() const
{
    return m_displacement;
}

void AxisTicker::setDisplacement(qreal newDisplacement)
{
    if (QtPrivate::fuzzyCompare(m_displacement, newDisplacement))
        return;
    m_displacement = newDisplacement;
    emit displacementChanged();
}

QColor AxisTicker::subTickColor() const
{
    return m_subTickColor;
}

void AxisTicker::setSubTickColor(QColor newSubTickColor)
{
    if (m_subTickColor == newSubTickColor)
        return;
    m_subTickColor = newSubTickColor;
    emit subTickColorChanged();
}

QColor AxisTicker::tickColor() const
{
    return m_tickColor;
}

void AxisTicker::setTickColor(QColor newTickColor)
{
    if (m_tickColor == newTickColor)
        return;
    m_tickColor = newTickColor;
    emit tickColorChanged();
}

qreal AxisTicker::subTickLineWidth() const
{
    return m_subTickLineWidth;
}

void AxisTicker::setSubTickLineWidth(qreal newSubTickLineWidth)
{
    if (QtPrivate::fuzzyCompare(m_subTickLineWidth, newSubTickLineWidth))
        return;
    m_subTickLineWidth = newSubTickLineWidth;
    emit subTickLineWidthChanged();
}

qreal AxisTicker::tickLineWidth() const
{
    return m_tickLineWidth;
}

void AxisTicker::setTickLineWidth(qreal newTickLineWidth)
{
    if (QtPrivate::fuzzyCompare(m_tickLineWidth, newTickLineWidth))
        return;
    m_tickLineWidth = newTickLineWidth;
    emit tickLineWidthChanged();
}

qreal AxisTicker::subTickScale() const
{
    return m_subTickScale;
}

void AxisTicker::setSubTickScale(qreal newSubTickScale)
{
    if (QtPrivate::fuzzyCompare(m_subTickScale, newSubTickScale))
        return;
    m_subTickScale = newSubTickScale;
    emit subTickScaleChanged();
}

qreal AxisTicker::subTickLength() const
{
    return m_subTickLength;
}

void AxisTicker::setSubTickLength(qreal newSubTickLength)
{
    if (QtPrivate::fuzzyCompare(m_subTickLength, newSubTickLength))
        return;
    m_subTickLength = newSubTickLength;
    emit subTickLengthChanged();
}

bool AxisTicker::isHorizontal() const
{
    return m_isHorizontal;
}

void AxisTicker::setIsHorizontal(bool newIsHorizontal)
{
    if (m_isHorizontal == newIsHorizontal)
        return;
    m_isHorizontal = newIsHorizontal;
    setupShaders();
    emit isHorizontalChanged();
}

bool AxisTicker::isFlipped() const
{
    return m_flipped;
}

void AxisTicker::setFlipped(bool newFlipped)
{
    if (m_flipped == newFlipped)
        return;
    m_flipped = newFlipped;
    emit flippedChanged();
}

QT_END_NAMESPACE

#include "moc_axisticker_p.cpp"

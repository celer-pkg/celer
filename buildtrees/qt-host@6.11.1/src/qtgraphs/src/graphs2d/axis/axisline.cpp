// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "axisline_p.h"

QT_BEGIN_NAMESPACE

AxisLine::AxisLine(QQuickItem *parent) :
      QQuickShaderEffect(parent)
{

}

AxisLine::~AxisLine() {}

void AxisLine::componentComplete()
{
    QQuickShaderEffect::componentComplete();
    setupShaders();
}

void AxisLine::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    m_iResolution = QVector3D(newGeometry.width(), newGeometry.height(), 1.0);
    emit iResolutionChanged();

    QQuickShaderEffect::geometryChange(newGeometry, oldGeometry);
}

void AxisLine::setupShaders()
{
    if (m_isHorizontal) {
        setFragmentShader(QUrl(QStringLiteral("qrc:/shaders/lineshaderhorizontal.frag.qsb")));
        setVertexShader(QUrl(QStringLiteral("qrc:/shaders/lineshaderhorizontal.vert.qsb")));
    } else {
        setFragmentShader(QUrl(QStringLiteral("qrc:/shaders/lineshadervertical.frag.qsb")));
        setVertexShader(QUrl(QStringLiteral("qrc:/shaders/lineshadervertical.vert.qsb")));
    }
}

QVector3D AxisLine::iResolution() const
{
    return m_iResolution;
}

qreal AxisLine::smoothing() const
{
    return m_smoothing;
}

void AxisLine::setSmoothing(qreal newSmoothing)
{
    if (QtPrivate::fuzzyCompare(m_smoothing, newSmoothing))
        return;
    m_smoothing = newSmoothing;
    emit smoothingChanged();
}

QColor AxisLine::color() const
{
    return m_color;
}

void AxisLine::setColor(QColor newColor)
{
    if (m_color == newColor)
        return;
    m_color = newColor;
    emit colorChanged();
}

qreal AxisLine::lineWidth() const
{
    return m_lineWidth;
}

void AxisLine::setLineWidth(qreal newLineWidth)
{
    if (QtPrivate::fuzzyCompare(m_lineWidth, newLineWidth))
        return;
    m_lineWidth = newLineWidth;
    emit lineWidthChanged();
}

bool AxisLine::isHorizontal() const
{
    return m_isHorizontal;
}

void AxisLine::setIsHorizontal(bool newIsHorizontal)
{
    if (m_isHorizontal == newIsHorizontal)
        return;
    m_isHorizontal = newIsHorizontal;
    emit isHorizontalChanged();
}

QT_END_NAMESPACE

#include "moc_axisline_p.cpp"

// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "colorgradient_p.h"

QT_BEGIN_NAMESPACE

ColorGradientStop::ColorGradientStop(QObject *parent)
    : QObject(parent)
{
}

qreal ColorGradientStop::position() const
{
    return m_position;
}

void ColorGradientStop::setPosition(qreal position)
{
    m_position = position;
    updateGradient();
    emit positionChanged(position);
}

QColor ColorGradientStop::color() const
{
    return m_color;
}

void ColorGradientStop::setColor(const QColor &color)
{
    m_color = color;
    updateGradient();
    emit colorChanged(color);
}

void ColorGradientStop::updateGradient()
{
    if (ColorGradient *grad = qobject_cast<ColorGradient*>(parent()))
        grad->doUpdate();
}

ColorGradient::ColorGradient(QObject *parent)
: QObject(parent)
{
}

ColorGradient::~ColorGradient()
{
}

QQmlListProperty<ColorGradientStop> ColorGradient::stops()
{
    return QQmlListProperty<ColorGradientStop>(this, &m_stops);
}

void ColorGradient::doUpdate()
{
    emit updated();
}

QT_END_NAMESPACE

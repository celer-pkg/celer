// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgcsseasing_p.h"
#include <QtCore/qmath.h>
#include <QtCore/qminmax.h>

QT_BEGIN_NAMESPACE

QSvgCssEasing::QSvgCssEasing(QSvgCssValues::EasingFunction easingFunction)
    : m_easingFunction(easingFunction)
{
}

QSvgCssValues::EasingFunction QSvgCssEasing::easingFunction() const
{
    return m_easingFunction;
}

QSvgCssCubicBezierEasing::QSvgCssCubicBezierEasing(QSvgCssValues::EasingFunction easingFunction, const QPointF &c1, const QPointF &c2)
    : QSvgCssEasing(easingFunction)
    , m_c1(c1)
    , m_c2(c2)
{
    m_easingCurve.setType(QEasingCurve::BezierSpline);
    m_easingCurve.addCubicBezierSegment(c1, c2, QPointF(1, 1));
}

qreal QSvgCssCubicBezierEasing::progress(qreal t)
{
    return m_easingCurve.valueForProgress(t);
}

QPointF QSvgCssCubicBezierEasing::c1() const
{
    return m_c1;
}

QPointF QSvgCssCubicBezierEasing::c2() const
{
    return m_c2;
}

QSvgCssStepsEasing::QSvgCssStepsEasing(quint32 stops, QSvgCssValues::StepPosition position)
    : QSvgCssEasing(QSvgCssValues::EasingFunction::Steps)
    , m_stops(qMax(quint32(1), stops))
    , m_stepPosition(position)
{
}

qreal QSvgCssStepsEasing::progress(qreal t)
{
    const qreal incr = 1.0 / m_stops;
    const qreal interval = (m_stepPosition == QSvgCssValues::StepPosition::JumpEnd) ?
                               qFloor(t / incr) : qCeil(t / incr);

    return interval * incr;
}

QSvgCssValues::StepPosition QSvgCssStepsEasing::stepPosition()
{
    return m_stepPosition;
}

QT_END_NAMESPACE

// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgabstractanimation_p.h"

QT_BEGIN_NAMESPACE

QSvgAbstractAnimation::QSvgAbstractAnimation()
    : m_start(0)
    , m_duration(0)
    , m_finished(false)
    , m_iterationCount(0)
{
}

QSvgAbstractAnimation::~QSvgAbstractAnimation()
{
    for (const QSvgAbstractAnimatedProperty *prop : std::as_const(m_properties))
        delete prop;
}

void QSvgAbstractAnimation::appendProperty(QSvgAbstractAnimatedProperty *property)
{
    m_properties.append(property);
}

QList<QSvgAbstractAnimatedProperty *> QSvgAbstractAnimation::properties() const
{
    return m_properties;
}

bool QSvgAbstractAnimation::finished() const
{
    return m_finished;
}

bool QSvgAbstractAnimation::isActive() const
{
    return !finished();
}

void QSvgAbstractAnimation::evaluateAnimation(qreal elapsedTime)
{
    if (m_duration == 0 || m_start > elapsedTime)
        return;

    qreal fractionOfTotalTime = (elapsedTime - m_start) / m_duration;

    if (m_iterationCount >= 0 && m_iterationCount < fractionOfTotalTime)
        m_finished = true;
    else
        m_finished = false;

    if (m_finished)
        return;

    qreal fractionOfCurrentIterationTime = fractionOfTotalTime - std::trunc(fractionOfTotalTime);

    for (const QSvgAbstractAnimatedProperty *animProperty : std::as_const(m_properties)) {
        const QList<qreal> keyFrames = animProperty->keyFrames();
        for (int i = 1; i < keyFrames.size(); i++) {
            qreal from = keyFrames.at(i - 1);
            qreal to = keyFrames.at(i);
            if (fractionOfCurrentIterationTime >= from && fractionOfCurrentIterationTime < to) {
                qreal currFraction = (fractionOfCurrentIterationTime - from) / (to - from);
                qreal effectiveFraction = m_easing->progress(currFraction);
                animProperty->interpolate(i, effectiveFraction);
            }
        }
    }
}

void QSvgAbstractAnimation::setRunningTime(int startMs, int durationMs)
{
    m_start = (startMs > 0) ? startMs : 0;
    m_duration = (durationMs > 0) ? durationMs : 0;
}

int QSvgAbstractAnimation::start() const
{
    return m_start;
}

int QSvgAbstractAnimation::duration() const
{
    return m_duration;
}

void QSvgAbstractAnimation::setIterationCount(int count)
{
    m_iterationCount = count;
}

int QSvgAbstractAnimation::iterationCount() const
{
    return m_iterationCount;
}

void QSvgAbstractAnimation::setEasing(QSvgEasingInterfacePtr easing)
{
    m_easing = std::move(easing);
}

QSvgEasingInterface *QSvgAbstractAnimation::easing() const
{
    return m_easing.get();
}

QT_END_NAMESPACE

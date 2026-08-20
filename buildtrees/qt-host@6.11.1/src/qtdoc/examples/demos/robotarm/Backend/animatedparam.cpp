// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "animatedparam.h"

#include <QVariantAnimation>

AnimatedParam::AnimatedParam(QObject *parent) : QVariantAnimation(parent)
{
    setDuration(1500);
    setEasingCurve(QEasingCurve::Linear);

    connect(this, &QVariantAnimation::valueChanged, this, &AnimatedParam::valueChanged);
    connect(this, &QAbstractAnimation::stateChanged, this, [this](QAbstractAnimation::State newState, QAbstractAnimation::State) {
        m_isRunning = (newState == QAbstractAnimation::Running);
    });
}

int AnimatedParam::value() const
{
    return currentValue().toInt();
}

void AnimatedParam::setValue(int newValue)
{
    if (newValue == endValue().toInt())
        return;

    setStartValue(value());
    setEndValue(newValue);
    start();
}

bool AnimatedParam::isRunning() const
{
    return m_isRunning;
}

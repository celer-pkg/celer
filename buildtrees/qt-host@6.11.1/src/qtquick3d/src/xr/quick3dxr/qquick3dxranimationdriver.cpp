// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxranimationdriver_p.h"

QT_BEGIN_NAMESPACE

QQuick3DXrAnimationDriver::QQuick3DXrAnimationDriver()
{
}

void QQuick3DXrAnimationDriver::advance()
{
    m_elapsed += m_step;
    advanceAnimation();
}

qint64 QQuick3DXrAnimationDriver::elapsed() const
{
    return m_elapsed;
}

void QQuick3DXrAnimationDriver::setStep(int stepSize)
{
    m_step = stepSize;
}

void QQuick3DXrAnimationDriver::start()
{
    m_elapsed = 0;
    QAnimationDriver::start();
}

QT_END_NAMESPACE

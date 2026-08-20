// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QPointF>
#include <QTransform>

#include "QtLottie/private/qlottierenderer_p.h"
#include "QtLottie/private/qlottiebasictransform_p.h"
#include "QtLottie/private/qlottieshapetransform_p.h"

QT_BEGIN_NAMESPACE

void QLottieRenderer::setTrimmingState(QLottieRenderer::TrimmingState trimmingState)
{
    m_trimmingState = trimmingState;
}

QLottieRenderer::TrimmingState QLottieRenderer::trimmingState() const
{
    return m_trimmingState;
}

void QLottieRenderer::saveTrimmingState()
{
    m_trimStateStack.push(m_trimmingState);
}

void QLottieRenderer::restoreTrimmingState()
{
    if (m_trimStateStack.size())
        m_trimmingState = m_trimStateStack.pop();
}

void QLottieRenderer::applyTransform(QTransform *xf, const QLottieBasicTransform &lottieXf, bool isShapeTransform)
{
    const QPointF pos = lottieXf.position();
    const qreal rot = lottieXf.rotation();
    const QPointF sca = lottieXf.scale();
    const QPointF anc = lottieXf.anchorPoint();

    xf->translate(pos.x(), pos.y());

    if (!qFuzzyIsNull(rot))
        xf->rotate(rot);

    if (isShapeTransform) {
        const QLottieShapeTransform &shxf = static_cast<const QLottieShapeTransform &>(lottieXf);
        if (!qFuzzyIsNull(shxf.skew())) {
            QTransform t(shxf.shearX(), shxf.shearY(), 0, -shxf.shearY(), shxf.shearX(), 0, 0, 0, 1);
            t *= QTransform(1, 0, 0, shxf.shearAngle(), 1, 0, 0, 0, 1);
            t *= QTransform(shxf.shearX(), -shxf.shearY(), 0, shxf.shearY(), shxf.shearX(), 0, 0, 0, 1);
            *xf = t * (*xf);
        }
    }

    xf->scale(sca.x(), sca.y());
    xf->translate(-anc.x(), -anc.y());
}

QT_END_NAMESPACE

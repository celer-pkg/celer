// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtLottie/private/qbeziereasing_p.h>

QT_BEGIN_NAMESPACE

void QBezierEasing::addCubicBezierSegment(const QPointF &c1, const QPointF &c2, const QPointF &endPoint)
{
    mBezier = QBezier::fromPoints(QPointF(0.0, 0.0), c1, c2, endPoint);
}

qreal QBezierEasing::valueForProgress(qreal progress) const
{
    return mBezier.pointAt(tForX(progress)).y();
}

qreal QBezierEasing::tForX(qreal x) const
{
    if (x <= 0.0)
        return 0.0;
    else if (x >= 1.0)
        return 1.0;

    qreal t0 = 0.0;
    qreal t1 = 1.0;

    for (int i = 0; i < 10; i++) {  // 10 iterations gives error smaller than 0.001
        qreal t = qreal(0.5) * (t0 + t1);
        qreal a, b, c, d;
        QBezier::coefficients(t, a, b, c, d);
        qreal xt = a * mBezier.x1 + b * mBezier.x2 + c * mBezier.x3 + d * mBezier.x4;
        if (xt < x)
            t0 = t;
        else
            t1 = t;
    }

    return t0;
}

QT_END_NAMESPACE

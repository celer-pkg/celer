// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "star.h"

#include <QtCore/qmath.h>

Star::Star()
    : m_theta(0),
      m_a(0),
      m_b(0),
      m_center(QVector2D(0.0f, 0.0f))
{
}

const void Star::calcXY()
{
    qreal &a = m_a;
    qreal &b = m_b;
    qreal &theta = m_theta;
    const QVector2D &p = m_center;

    qreal beta  = -m_angle;
    qreal alpha = qDegreesToRadians(theta);

    // temporaries to save cpu time
    qreal cosalpha = qCos(alpha);
    qreal sinalpha = qSin(alpha);
    qreal cosbeta  = qCos(beta);
    qreal sinbeta  = qSin(beta);

    m_pos = QVector2D(p.x() + (a * cosalpha * cosbeta - b * sinalpha * sinbeta),
                      p.y() + (a * cosalpha * sinbeta + b * sinalpha * cosbeta));
}

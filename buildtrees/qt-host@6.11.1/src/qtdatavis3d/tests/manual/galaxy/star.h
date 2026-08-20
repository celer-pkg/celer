// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef STAR_H
#define STAR_H

#include <QtCore/qglobal.h>
#include <QtGui/QVector2D>

class Star
{
public:
    Star();
    const void calcXY();

    qreal m_theta;    // position auf der ellipse
//    qreal m_velTheta; // angular velocity
    qreal m_angle;    // Schräglage der Ellipse
    qreal m_a;        // kleine halbachse
    qreal m_b;        // große halbachse
//    qreal m_temp;     // star temperature
//    qreal m_mag;      // brightness;
    QVector2D  m_center;   // center of the elliptical orbit
//    QVector2D  m_vel;      // Current velocity (calculated)
    QVector2D  m_pos;      // current position in kartesion koordinates
};

#endif // STAR_H

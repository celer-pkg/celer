// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qdoublevector2d_p.h"
#include "qdoublevector3d_p.h"
#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmath.h>

QT_BEGIN_NAMESPACE

QDoubleVector2D::QDoubleVector2D(const QDoubleVector3D &vector) :
    xp(vector.xp), yp(vector.yp)
{
}

double QDoubleVector2D::length() const
{
    return qHypot(xp, yp);
}

QDoubleVector2D QDoubleVector2D::normalized() const
{
    const double len = length();
    if (qFuzzyIsNull(len - 1.0))
        return *this;
    else if (!qFuzzyIsNull(len))
        return *this / len;
    else
        return QDoubleVector2D();
}

void QDoubleVector2D::normalize()
{
    double len = length();
    if (qFuzzyIsNull(len - 1.0) || qFuzzyIsNull(len))
        return;

    xp /= len;
    yp /= len;
}

QDoubleVector3D QDoubleVector2D::toVector3D() const
{
    return QDoubleVector3D(xp, yp, 0.0);
}

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug dbg, const QDoubleVector2D &vector)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QDoubleVector2D(" << vector.x() << ", " << vector.y() << ')';
    return dbg;
}

#endif

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &stream, const QDoubleVector2D &vector)
{
    stream << double(vector.x()) << double(vector.y());
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QDoubleVector2D &vector)
{
    double x, y;
    stream >> x;
    stream >> y;
    vector.setX(double(x));
    vector.setY(double(y));
    return stream;
}

#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE

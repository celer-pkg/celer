// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qdoublevector3d_p.h"
#include <QtCore/qdatastream.h>
#include <QtCore/qmath.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

QDoubleVector3D QDoubleVector3D::normalized() const
{
    const double len = length();
    if (qFuzzyIsNull(len - 1.0))
        return *this;
    else if (!qFuzzyIsNull(len))
        return *this / len;
    else
        return QDoubleVector3D();
}

void QDoubleVector3D::normalize()
{
    double len = length();
    if (qFuzzyIsNull(len) || qFuzzyIsNull(len - 1.0))
        return;

    xp /= len;
    yp /= len;
    zp /= len;
}

QDoubleVector3D QDoubleVector3D::normal(const QDoubleVector3D &v1, const QDoubleVector3D &v2)
{
    return crossProduct(v1, v2).normalized();
}

QDoubleVector3D QDoubleVector3D::normal
        (const QDoubleVector3D &v1, const QDoubleVector3D &v2, const QDoubleVector3D &v3)
{
    return crossProduct((v2 - v1), (v3 - v1)).normalized();
}

double QDoubleVector3D::distanceToPlane
    (const QDoubleVector3D &plane1, const QDoubleVector3D &plane2, const QDoubleVector3D &plane3) const
{
    QDoubleVector3D n = normal(plane2 - plane1, plane3 - plane1);
    return dotProduct(*this - plane1, n);
}

double QDoubleVector3D::distanceToLine
        (const QDoubleVector3D &point, const QDoubleVector3D &direction) const
{
    if (direction.isNull())
        return (*this - point).length();
    QDoubleVector3D p = point + dotProduct(*this - point, direction) * direction;
    return (*this - p).length();
}

double QDoubleVector3D::length() const
{
    return qHypot(xp, yp, zp);
}

#ifndef QT_NO_DEBUG_STREAM

QDebug operator<<(QDebug dbg, const QDoubleVector3D &vector)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QDoubleVector3D("
        << vector.x() << ", " << vector.y() << ", " << vector.z() << ')';
    return dbg;
}

#endif

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &stream, const QDoubleVector3D &vector)
{
    stream << double(vector.x()) << double(vector.y())
           << double(vector.z());
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QDoubleVector3D &vector)
{
    double x, y, z;
    stream >> x;
    stream >> y;
    stream >> z;
    vector.setX(double(x));
    vector.setY(double(y));
    vector.setZ(double(z));
    return stream;
}

#endif // QT_NO_DATASTREAM

QT_END_NAMESPACE

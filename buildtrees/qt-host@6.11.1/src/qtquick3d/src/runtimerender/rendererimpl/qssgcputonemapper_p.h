// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGCPUTONEMAPPER_P_H
#define QSSGCPUTONEMAPPER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qvector3d.h>

QT_BEGIN_NAMESPACE

namespace QSSGTonemapper {

inline QVector3D vsqrt(const QVector3D &a)
{
    return QVector3D(std::sqrt(a.x()), std::sqrt(a.y()), std::sqrt(a.z()));
}

inline QVector3D vmax(const QVector3D &a, const QVector3D &b)
{
    return QVector3D(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()));
}

inline QVector3D vclamp(const QVector3D &a, float b, float c)
{
    return QVector3D(qBound(b, a.x(), c), qBound(b, a.y(), c), qBound(b, a.z(), c));
}

inline QVector3D vadd(const QVector3D &a, float b)
{
    return QVector3D(a.x() + b, a.y() + b, a.z() + b);
}

template<typename mType>
inline QVector3D mad(mType m, const QVector3D &a, float b)
{
    return vadd(m * a, b);
}

// all the implementation here is expected to match tonemapper.glsllib

inline QVector3D tonemapLinearToSrgb(const QVector3D &c)
{
    const QVector3D S1 = vsqrt(c);
    const QVector3D S2 = vsqrt(S1);
    const QVector3D S3 = vsqrt(S2);
    return 0.58512238f * S1 + 0.78314035f * S2 - 0.36826273f * S3;
}

inline QVector3D tonemapHejlDawson(const QVector3D &c)
{
    const QVector3D cc = vmax(QVector3D(0.0f, 0.0f, 0.0f), vadd(c, -0.004f));
    return (cc * mad(6.2f, cc, 0.5f)) / vadd(cc * mad(6.2f, cc, 1.7f), 0.06f);
}

inline QVector3D tonemapAces(const QVector3D &c)
{
    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;
    return tonemapLinearToSrgb(vclamp((c * mad(A, c, B)) / vadd(c * mad(C, c, D), E), 0.0f, 1.0f));
}

inline QVector3D tonemapFilmicSub(const QVector3D &c)
{
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;
    return vadd(vadd(c * mad(A, c, C * B), D * E) / vadd(c * mad(A, c, B), D * F), -E / F);
}

inline QVector3D tonemapFilmic(const QVector3D &c)
{
    static const QVector3D whiteScale = QVector3D(1.0f, 1.0f, 1.0f) / tonemapFilmicSub(QVector3D(11.2f, 11.2f, 11.2f));
    return tonemapLinearToSrgb(tonemapFilmicSub(c * 2.0f) * whiteScale);
}

} // namespace QSSGTonemapper

QT_END_NAMESPACE

#endif

// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEUTILS_H
#define QQUICK3DPARTICLEUTILS_H

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

#include <qmath.h>
#include <qmatrix4x4.h>
#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuaternion;
class QQuick3DNode;
class QQuick3DModel;
class QQmlContext;

// Define this to use C++ std::sinf & std::cosf for particles.
// Those are more accurate, but also potentially less performant
// than the lookup table versions.
#ifdef QT_QUICK3D_PARTICLES_USE_STANDARD_SIN
#define QPSIN sinf
#define QPCOS cosf
#else
#define QPSIN qLookupSin
#define QPCOS qLookupCos
#endif

// These sin & cos implementations are copied from qmath.h qFastSin & qFastCos.
// Changes:
// - Modified to use float instead of qreal (double).
// - Use constexpr for optimization.
//
// With input values between 0 .. 2*M_PI, the accuracy of qLookupSin is quite good
// (max delta ~2.5e-06). When the input values grow, accuracy decreases. For example
// with value 2058, diff is ~0.00014 and with value 9632 diff is ~0.00074.
// So these methods should not be used with a large input range if accuracy is important.

#define QT_QUICK3D_SINE_TABLE_SIZE 256

// Precalculated constexpr helpers
static constexpr float QT_QUICK3D_SINE_H1 = 0.5f * float(QT_QUICK3D_SINE_TABLE_SIZE / M_PI);
static constexpr float QT_QUICK3D_SINE_H2 = 2.0f * float(M_PI / QT_QUICK3D_SINE_TABLE_SIZE);

extern Q_QUICK3DPARTICLES_EXPORT const float qt_quick3d_sine_table[QT_QUICK3D_SINE_TABLE_SIZE];

inline float qLookupSin(float x)
{
    int si = int(x * QT_QUICK3D_SINE_H1); // Would be more accurate with qRound, but slower.
    float d = x - si * QT_QUICK3D_SINE_H2;
    int ci = si + QT_QUICK3D_SINE_TABLE_SIZE / 4;
    si &= QT_QUICK3D_SINE_TABLE_SIZE - 1;
    ci &= QT_QUICK3D_SINE_TABLE_SIZE - 1;
    return qt_quick3d_sine_table[si] + (qt_quick3d_sine_table[ci] - 0.5f * qt_quick3d_sine_table[si] * d) * d;
}

inline float qLookupCos(float x)
{
    int ci = int(x * QT_QUICK3D_SINE_H1); // Would be more accurate with qRound, but slower.
    float d = x - ci * QT_QUICK3D_SINE_H2;
    int si = ci + QT_QUICK3D_SINE_TABLE_SIZE / 4;
    si &= QT_QUICK3D_SINE_TABLE_SIZE - 1;
    ci &= QT_QUICK3D_SINE_TABLE_SIZE - 1;
    return qt_quick3d_sine_table[si] - (qt_quick3d_sine_table[ci] + 0.5f * qt_quick3d_sine_table[si] * d) * d;
}

QQuick3DNode *getSharedParentNode(QQuick3DNode *node, QQuick3DNode *system);
QMatrix4x4 calculateParticleTransform(const QQuick3DNode *parent, const QQuick3DNode *systemSharedParent);
QQuaternion calculateParticleRotation(const QQuick3DNode *parent, const QQuick3DNode *systemSharedParent);
QList<QVector3D> positionsFromModel(QQuick3DModel *model, const QMatrix4x4 *matrix, QQmlContext *context);

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEUTILS_H

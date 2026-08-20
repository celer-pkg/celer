// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgbounds3_p.h"
#include <private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

void QSSGBounds3::include(const QVector3D &v)
{
    minimum = QSSGUtils::vec3::minimum(minimum, v);
    maximum = QSSGUtils::vec3::maximum(maximum, v);
}

void QSSGBounds3::include(const QSSGBounds3 &b)
{
    minimum = QSSGUtils::vec3::minimum(minimum, b.minimum);
    maximum = QSSGUtils::vec3::maximum(maximum, b.maximum);
}

bool QSSGBounds3::isFinite() const
{
    return QSSGUtils::vec3::isFinite(minimum) && QSSGUtils::vec3::isFinite(maximum);
}

QSSGBounds3 QSSGBounds3::boundsOfPoints(const QVector3D &v0, const QVector3D &v1)
{
    return QSSGBounds3(QSSGUtils::vec3::minimum(v0, v1), QSSGUtils::vec3::maximum(v0, v1));
}

QSSGBounds3 QSSGBounds3::basisExtent(const QVector3D &center, const QMatrix3x3 &basis, const QVector3D &extent)
{
    // extended basis vectors
    QVector3D c0 = QVector3D(basis(0, 0), basis(1, 0), basis(2, 0)) * extent.x();
    QVector3D c1 = QVector3D(basis(0, 1), basis(1, 1), basis(2, 1)) * extent.y();
    QVector3D c2 = QVector3D(basis(0, 2), basis(1, 2), basis(2, 2)) * extent.z();

    QVector3D w;
    // find combination of base vectors that produces max. distance for each component = sum of
    // abs()
    w.setX(qAbs(c0.x()) + qAbs(c1.x()) + qAbs(c2.x()));
    w.setY(qAbs(c0.y()) + qAbs(c1.y()) + qAbs(c2.y()));
    w.setZ(qAbs(c0.z()) + qAbs(c1.z()) + qAbs(c2.z()));

    return QSSGBounds3(center - w, center + w);
}

QSSGBounds3 QSSGBounds3::transform(const QMatrix3x3 &matrix, const QSSGBounds3 &bounds)
{
    return bounds.isEmpty()
            ? bounds
            : QSSGBounds3::basisExtent(QSSGUtils::mat33::transform(matrix, bounds.center()), matrix, bounds.extents());
}

void QSSGBounds3::transform(const QMatrix4x4 &inMatrix)
{
    if (!isEmpty()) {
        const auto pointsPrevious = toQSSGBoxPointsNoEmptyCheck();
        setEmpty();
        for (const QVector3D& point : pointsPrevious)
            include(inMatrix.map(point));
    }
}

QVector3D QSSGBounds3::getSupport(const QVector3D &direction) const
{
    const QVector3D halfExtents = extents();
    return QVector3D(direction.x() > 0 ? halfExtents.x() : -halfExtents.x(),
                     direction.y() > 0 ? halfExtents.y() : -halfExtents.y(),
                     direction.z() > 0 ? halfExtents.z() : -halfExtents.z()) + center();
}

QT_END_NAMESPACE

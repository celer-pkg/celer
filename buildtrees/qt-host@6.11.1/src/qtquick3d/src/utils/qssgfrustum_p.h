// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGFRUSTUM_H
#define QSSGFRUSTUM_H

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

#include <QtQuick3DUtils/private/qssgplane_p.h>
#include <QtQuick3DUtils/private/qssgbounds3_p.h>

QT_BEGIN_NAMESPACE

/**
\brief Representation of a 6 plane frustum.

*/
class QSSGFrustum
{
public:
    /**
    \brief Performs AABB test on bounds
    */
    Q_ALWAYS_INLINE bool contains(const QSSGBounds3 &box) const;

    QSSGPlane m_planes[6]; //!< The six planes of frustum
};

Q_ALWAYS_INLINE bool QSSGFrustum::contains(const QSSGBounds3 &box) const
{
    for (const auto &plane : m_planes) {
        QVector3D p(plane.n.x() > 0 ? box.maximum.x() : box.minimum.x(),
                    plane.n.y() > 0 ? box.maximum.y() : box.minimum.y(),
                    plane.n.z() > 0 ? box.maximum.z() : box.minimum.z());
        if (plane.distance(p) < 0)
            return false;
    }
    return true;
}

QT_END_NAMESPACE

#endif // QSSGFRUSTUM_H

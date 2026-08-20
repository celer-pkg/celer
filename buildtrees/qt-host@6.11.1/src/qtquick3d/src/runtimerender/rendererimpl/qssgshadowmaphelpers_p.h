// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSHADOWMAPHELPERS_P_H
#define QSSGSHADOWMAPHELPERS_P_H

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

#include <QVector3D>
#include <QList>
#include <array>

QT_BEGIN_NAMESPACE

using QSSGBoxPoints = std::array<QVector3D, 8>;
class QSSGBounds3;
class QSSGDebugDrawSystem;
class QColor;
class QMatrix4x4;

namespace ShadowmapHelpers {
void addDebugFrustum(const QSSGBoxPoints &frustumPoints, const QColor &color, QSSGDebugDrawSystem *debugDrawSystem);
void addDebugBox(const QSSGBoxPoints &boxUnsorted, const QColor &color, QSSGDebugDrawSystem *debugDrawSystem);
void addDirectionalLightDebugBox(const QSSGBoxPoints &box, QSSGDebugDrawSystem *debugDrawSystem);

QList<QVector3D> intersectBoxByFrustum(const QSSGBoxPoints &frustumPoints,
                                       const QSSGBoxPoints &box,
                                       QSSGDebugDrawSystem *debugDrawSystem,
                                       const QColor &color);

QList<QVector3D> intersectBoxByBox(const QSSGBounds3 &boxBounds, const QSSGBoxPoints &box);
}

QT_END_NAMESPACE

#endif // QSSGSHADOWMAPHELPERS_P_H

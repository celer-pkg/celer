// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_REFLECTION_PROBE_H
#define QSSG_RENDER_REFLECTION_PROBE_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendererutil_p.h>
#include <QColor>

QT_BEGIN_NAMESPACE

struct QSSGRenderImage;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderReflectionProbe : public QSSGRenderNode
{
    enum class ReflectionRefreshMode
    {
        FirstFrame,
        EveryFrame
    };

    enum class ReflectionTimeSlicing
    {
        None,
        AllFacesAtOnce,
        IndividualFaces
    };

    quint32 reflectionMapRes = 8;
    QColor clearColor = Qt::transparent;
    ReflectionRefreshMode refreshMode = ReflectionRefreshMode::FirstFrame;
    ReflectionTimeSlicing timeSlicing = ReflectionTimeSlicing::None;
    bool parallaxCorrection = false;
    QVector3D boxSize { 0.0, 0.0, 0.0 };
    QVector3D boxOffset { 0.0, 0.0, 0.0 };
    bool hasScheduledUpdate = false;
    QSSGRenderImage *texture = nullptr;

    explicit QSSGRenderReflectionProbe();
};

QT_END_NAMESPACE

#endif

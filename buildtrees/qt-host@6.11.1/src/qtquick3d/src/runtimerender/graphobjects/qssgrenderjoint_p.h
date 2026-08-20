// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_JOINT_H
#define QSSG_RENDER_JOINT_H

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

QT_BEGIN_NAMESPACE

struct QSSGRenderSkeleton;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderJoint : public QSSGRenderNode
{
    Q_DISABLE_COPY(QSSGRenderJoint)

    int index;
    QSSGRenderSkeleton *skeletonRoot = nullptr;

    QSSGRenderJoint();
    ~QSSGRenderJoint();
};
QT_END_NAMESPACE

#endif

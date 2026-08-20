// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_SKELETON_H
#define QSSG_RENDER_SKELETON_H

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
#include <QtQuick3DRuntimeRender/private/qssgrendertexturedata_p.h>

QT_BEGIN_NAMESPACE

class QSSGRenderTextureData;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderSkeleton : public QSSGRenderNode
{
    explicit QSSGRenderSkeleton();
    ~QSSGRenderSkeleton();
    Q_DISABLE_COPY(QSSGRenderSkeleton)

    int maxIndex = -1;

    bool boneTransformsDirty = false;
    bool skinningDirty = false;
    bool containsNonJointNodes = false;
    QByteArray boneData;
    QSSGRenderTextureData boneTexData;
    quint32 boneCount = 0;
};
QT_END_NAMESPACE

#endif

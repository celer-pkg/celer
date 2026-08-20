// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_ITEM2D_H
#define QSSG_RENDER_ITEM2D_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QSGNode;
class QSGRootNode;
class QRhiRenderPassDescriptor;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderItem2D : public QSSGRenderNode
{
    Q_DISABLE_COPY(QSSGRenderItem2D)

    QSGRootNode *m_rootNode = nullptr;

    QSSGRenderItem2D();
    ~QSSGRenderItem2D() override;

    QSSGRenderItem2DHandle ih;

    QPointer<QObject> m_frontEndObject;
};
QT_END_NAMESPACE

#endif

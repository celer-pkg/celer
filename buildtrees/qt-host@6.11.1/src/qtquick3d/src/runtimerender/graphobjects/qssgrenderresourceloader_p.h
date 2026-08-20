// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERRESOURCELOADER_H
#define QSSGRENDERRESOURCELOADER_H

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
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderResourceLoader : public QSSGRenderGraphObject
{
    QVector<QSSGRenderGraphObject *> geometries;
    QVector<QSSGRenderGraphObject *> textures;
    QVector<QSSGRenderPath> meshes;

    QSSGRenderResourceLoader();
};

QT_END_NAMESPACE

#endif // QSSGRENDERRESOURCELOADER_H

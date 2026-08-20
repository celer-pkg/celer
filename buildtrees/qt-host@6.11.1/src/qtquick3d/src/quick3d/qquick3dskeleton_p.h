// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSKELETON_H
#define QSSGSKELETON_H

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

#include <QtQuick3D/private/qquick3dnode_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DSkeleton : public QQuick3DNode
{
    Q_OBJECT

    QML_NAMED_ELEMENT(Skeleton)

public:
    explicit QQuick3DSkeleton(QQuick3DNode *parent = nullptr);
    ~QQuick3DSkeleton() override;

public Q_SLOTS:

Q_SIGNALS:
    void skeletonNodeDirty();

private:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
};

QT_END_NAMESPACE

#endif // QSSGSKELETON_H

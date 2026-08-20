// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGJOINT_H
#define QSSGJOINT_H

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
#include <QtQuick3D/private/qquick3dskeleton_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderskeleton_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DJoint : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(qint32 index READ index WRITE setIndex NOTIFY indexChanged)
    Q_PROPERTY(QQuick3DSkeleton *skeletonRoot READ skeletonRoot WRITE setSkeletonRoot NOTIFY skeletonRootChanged)

    QML_NAMED_ELEMENT(Joint)

public:
    explicit QQuick3DJoint(QQuick3DNode *parent = nullptr);
    ~QQuick3DJoint() override;

    qint32 index() const;
    QQuick3DSkeleton *skeletonRoot() const;

public Q_SLOTS:
    void setIndex(qint32 index);
    void setSkeletonRoot(QQuick3DSkeleton *skeleton);

Q_SIGNALS:
    void indexChanged();
    void skeletonRootChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;

private Q_SLOTS:

private:
    bool m_indexDirty = true;
    bool m_skeletonRootDirty = true;
    int m_index = 0;

    QQuick3DSkeleton *m_skeletonRoot = nullptr;

    QMetaObject::Connection m_skeletonConnection;
};

QT_END_NAMESPACE

#endif // QSSGJOINT_H

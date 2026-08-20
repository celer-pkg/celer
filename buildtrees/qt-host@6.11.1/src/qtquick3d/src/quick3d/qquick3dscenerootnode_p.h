// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DSCENEROOTNODE_P_H
#define QQUICK3DSCENEROOTNODE_P_H

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
class QQuick3DViewport;

class QQuick3DSceneRootNode : public QQuick3DNode
{
    Q_OBJECT
public:
    explicit QQuick3DSceneRootNode(QQuick3DViewport *view3D = nullptr, QQuick3DNode *parent = nullptr);
    ~QQuick3DSceneRootNode() override;

    QQuick3DViewport *view3D();

private:
    QQuick3DViewport *m_view3D = nullptr;

protected:
    virtual QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) final;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuick3DSceneRootNode)

#endif // QQUICK3DSCENEROOTNODE_P_H

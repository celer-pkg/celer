// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DSIMPLEQUADRENDERPASS_P_H
#define QQUICK3DSIMPLEQUADRENDERPASS_P_H

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

#include <QQuick3DRenderExtension>

#include <QtQuick3D/private/qquick3dshaderutils_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DSimpleQuadRenderer : public QQuick3DRenderExtension
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DTexture *texture READ texture WRITE setTexture NOTIFY textureChanged FINAL)
    QML_NAMED_ELEMENT(SimpleQuadRenderer)
    QML_ADDED_IN_VERSION(6, 11)
public:
    QQuick3DSimpleQuadRenderer();
    QQuick3DTexture *texture() const;
    void setTexture(QQuick3DTexture *newSource);

signals:
    void textureChanged();

protected:
    virtual QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) final;
    void itemChange(ItemChange, const ItemChangeData &) final;

private:
    void updateSceneManager(QQuick3DSceneManager *sceneManager);

    QQuick3DTexture *m_source = nullptr;
};

QT_END_NAMESPACE

#endif // QQUICK3DSIMPLEQUADRENDERPASS_P_H

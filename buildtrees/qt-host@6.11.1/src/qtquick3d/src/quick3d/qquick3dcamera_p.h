// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGCAMERA_H
#define QSSGCAMERA_H

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

class QSSGRenderCamera;
class Q_QUICK3D_EXPORT QQuick3DCamera : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(bool frustumCullingEnabled READ frustumCullingEnabled WRITE setFrustumCullingEnabled NOTIFY frustumCullingEnabledChanged)
    Q_PROPERTY(QQuick3DNode *lookAtNode READ lookAtNode WRITE setLookAtNode NOTIFY lookAtNodeChanged REVISION(6, 4))
    Q_PROPERTY(float levelOfDetailBias READ levelOfDetailBias WRITE setLevelOfDetailBias NOTIFY levelOfDetailBiasChanged REVISION(6, 5))
    QML_NAMED_ELEMENT(Camera)
    QML_UNCREATABLE("Camera is Abstract")
public:
    Q_INVOKABLE QVector3D mapToViewport(const QVector3D &scenePos) const;
    Q_INVOKABLE QVector3D mapFromViewport(const QVector3D &viewportPos) const;
    QVector3D mapToViewport(const QVector3D &scenePos,
                            qreal width,
                            qreal height);
    QVector3D mapFromViewport(const QVector3D &viewportPos,
                              qreal width,
                              qreal height);

    Q_INVOKABLE void lookAt(const QVector3D &scenePos);
    Q_INVOKABLE void lookAt(QQuick3DNode *node);

    // It will be used only after the scene was drawn.
    // It means that the spatialNode of this camera already was created.
    void updateGlobalVariables(const QRectF &inViewport);

    bool frustumCullingEnabled() const;
    QQuick3DNode *lookAtNode() const;
    Q_REVISION(6, 5) float levelOfDetailBias() const;

public Q_SLOTS:
    void setFrustumCullingEnabled(bool frustumCullingEnabled);
    void setLookAtNode(QQuick3DNode *node);
    Q_REVISION(6, 5) void setLevelOfDetailBias(float newLevelOFDetailBias);

Q_SIGNALS:
    void frustumCullingEnabledChanged();
    Q_REVISION(6, 4) void lookAtNodeChanged();
    Q_REVISION(6, 5) void levelOfDetailBiasChanged();

protected:
    explicit QQuick3DCamera(QQuick3DNodePrivate &dd, QQuick3DNode *parent = nullptr);

    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private Q_SLOTS:
    void updateLookAt();

private:
    bool m_frustumCullingEnabled = false;
    QQuick3DNode *m_lookAtNode = nullptr;
    float m_levelOfDetailBias = 1.0f;
};

QT_END_NAMESPACE

#endif // QSSGCAMERA_H

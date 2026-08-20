// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ABSTRACTCOLLISIONSHAPE_H
#define ABSTRACTCOLLISIONSHAPE_H

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

#include <QtQuick3DPhysics/qtquick3dphysicsglobal.h>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQml/QQmlEngine>

namespace physx {
class PxGeometry;
}

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QAbstractCollisionShape : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(bool enableDebugDraw READ enableDebugDraw WRITE setEnableDebugDraw NOTIFY
                       enableDebugDrawChanged)
    QML_NAMED_ELEMENT(CollisionShape)
    QML_UNCREATABLE("abstract interface")
public:
    explicit QAbstractCollisionShape(QQuick3DNode *parent = nullptr);
    virtual ~QAbstractCollisionShape();

    virtual physx::PxGeometry *getPhysXGeometry() = 0;
    bool enableDebugDraw() const;

    virtual bool isStaticShape() const = 0;

public slots:
    void setEnableDebugDraw(bool enableDebugDraw);

signals:
    void enableDebugDrawChanged(bool enableDebugDraw);
    void needsRebuild(QObject *);

protected:
    bool m_scaleDirty = true;
    QVector3D m_prevScale;

private slots:
    void handleScaleChange();

private:
    bool m_enableDebugDraw = false;
};

QT_END_NAMESPACE

#endif // ABSTRACTCOLLISIONSHAPE_H

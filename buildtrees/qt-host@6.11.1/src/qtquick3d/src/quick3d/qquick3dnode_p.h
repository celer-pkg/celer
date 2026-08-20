// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGNODE_H
#define QSSGNODE_H

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

#include <QtQuick3D/qquick3dobject.h>

#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtGui/QMatrix4x4>

QT_BEGIN_NAMESPACE
struct QSSGRenderNode;
class QQuick3DNodePrivate;
class Q_QUICK3D_EXPORT QQuick3DNode : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(float x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(float y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(float z READ z WRITE setZ NOTIFY zChanged)
    Q_PROPERTY(QQuaternion rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(QVector3D eulerRotation READ eulerRotation WRITE setEulerRotation NOTIFY eulerRotationChanged)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(QVector3D scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(QVector3D pivot READ pivot WRITE setPivot NOTIFY pivotChanged)
    Q_PROPERTY(float opacity READ localOpacity WRITE setLocalOpacity NOTIFY localOpacityChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QVector3D forward READ forward NOTIFY forwardChanged)
    Q_PROPERTY(QVector3D up READ up NOTIFY upChanged)
    Q_PROPERTY(QVector3D right READ right NOTIFY rightChanged)
    Q_PROPERTY(QVector3D scenePosition READ scenePosition NOTIFY scenePositionChanged)
    Q_PROPERTY(QQuaternion sceneRotation READ sceneRotation NOTIFY sceneRotationChanged)
    Q_PROPERTY(QVector3D sceneScale READ sceneScale NOTIFY sceneScaleChanged)
    Q_PROPERTY(QMatrix4x4 sceneTransform READ sceneTransform NOTIFY sceneTransformChanged)
    Q_PROPERTY(int staticFlags READ staticFlags WRITE setStaticFlags NOTIFY staticFlagsChanged)
    Q_PROPERTY(int layers READ layers WRITE setLayers NOTIFY layersChanged REVISION(6, 11))

    QML_NAMED_ELEMENT(Node)

public:
    enum TransformSpace {
        LocalSpace,
        ParentSpace,
        SceneSpace
    };
    Q_ENUM(TransformSpace)

    enum StaticFlags {
        None
    };
    Q_ENUM(StaticFlags)

    explicit QQuick3DNode(QQuick3DNode *parent = nullptr);
    ~QQuick3DNode() override;

    float x() const;
    float y() const;
    float z() const;
    QQuaternion rotation() const;
    QVector3D eulerRotation() const;
    QVector3D position() const;
    QVector3D scale() const;
    QVector3D pivot() const;
    float localOpacity() const;
    bool visible() const;
    int staticFlags() const;

    QQuick3DNode *parentNode() const;

    QVector3D forward() const;
    QVector3D up() const;
    QVector3D right() const;

    QVector3D scenePosition() const;
    QQuaternion sceneRotation() const;
    QVector3D sceneScale() const;
    QMatrix4x4 sceneTransform() const;

    Q_INVOKABLE void rotate(qreal degrees, const QVector3D &axis, QQuick3DNode::TransformSpace space);

    Q_INVOKABLE QVector3D mapPositionToScene(const QVector3D &localPosition) const;
    Q_INVOKABLE QVector3D mapPositionFromScene(const QVector3D &scenePosition) const;
    Q_INVOKABLE QVector3D mapPositionToNode(const QQuick3DNode *node, const QVector3D &localPosition) const;
    Q_INVOKABLE QVector3D mapPositionFromNode(const QQuick3DNode *node, const QVector3D &localPosition) const;
    Q_INVOKABLE QVector3D mapDirectionToScene(const QVector3D &localDirection) const;
    Q_INVOKABLE QVector3D mapDirectionFromScene(const QVector3D &sceneDirection) const;
    Q_INVOKABLE QVector3D mapDirectionToNode(const QQuick3DNode *node, const QVector3D &localDirection) const;
    Q_INVOKABLE QVector3D mapDirectionFromNode(const QQuick3DNode *node, const QVector3D &localDirection) const;

    void markAllDirty() override;

    int layers() const;

protected:
    void connectNotify(const QMetaMethod &signal) override;
    void disconnectNotify(const QMetaMethod &signal) override;
    void componentComplete() override;

public Q_SLOTS:
    void setX(float x);
    void setY(float y);
    void setZ(float z);
    void setRotation(const QQuaternion &rotation);
    void setEulerRotation(const QVector3D &eulerRotation);
    void setPosition(const QVector3D &position);
    void setScale(const QVector3D &scale);
    void setPivot(const QVector3D &pivot);
    void setLocalOpacity(float opacity);
    void setVisible(bool visible);
    void setStaticFlags(int staticFlags);
    void setLayers(int newLayers);

Q_SIGNALS:
    void xChanged();
    void yChanged();
    void zChanged();
    void rotationChanged();
    void eulerRotationChanged();
    void positionChanged();
    void scaleChanged();
    void pivotChanged();
    void localOpacityChanged();
    void visibleChanged();
    void forwardChanged();
    void upChanged();
    void rightChanged();
    void sceneTransformChanged();
    void scenePositionChanged();
    void sceneRotationChanged();
    void sceneScaleChanged();
    void staticFlagsChanged();

    void layersChanged();

protected:
    QQuick3DNode(QQuick3DNodePrivate &dd, QQuick3DNode *parent = nullptr);
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    virtual void itemChange(ItemChange, const ItemChangeData &) override;

private:
    friend QQuick3DSceneManager;
    Q_DISABLE_COPY(QQuick3DNode)
    Q_DECLARE_PRIVATE(QQuick3DNode)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuick3DNode)

#endif // QSSGNODE_H

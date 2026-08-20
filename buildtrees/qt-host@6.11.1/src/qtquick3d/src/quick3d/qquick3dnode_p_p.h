// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DNODE_P_P_H
#define QQUICK3DNODE_P_P_H

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


#include <QtQuick3D/private/qtquick3dglobal_p.h>

#include "qquick3dobject_p.h"
#include "qquick3dnode_p.h"
#include "qquick3dcontentlayer_p.h"

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtGui/QMatrix4x4>

QT_BEGIN_NAMESPACE

class QQuick3DNode;

class Q_QUICK3D_EXPORT QQuick3DNodePrivate : public QQuick3DObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuick3DNode)

public:

    explicit QQuick3DNodePrivate(QQuick3DObjectPrivate::Type t,
                                 QQuick3DContentLayer::LayerFlag layerFlag = QQuick3DContentLayer::Layer0);
    ~QQuick3DNodePrivate();
    void init();

    void calculateGlobalVariables();
    void markSceneTransformDirty();

    inline QMatrix4x4 localRotationMatrix() const;
    inline QMatrix4x4 sceneRotationMatrix() const;

    void emitChangesToSceneTransform();
    bool isSceneTransformRelatedSignal(const QMetaMethod &signal) const;
    bool isDirectionRelatedSignal(const QMetaMethod &signal) const;

    void setIsHiddenInEditor(bool isHidden);

    static inline QQuick3DNodePrivate *get(QQuick3DNode *node) { return node->d_func(); }

    void setLocalTransform(const QMatrix4x4 &transform);

    RotationData m_rotation;
    QVector3D m_position;
    QVector3D m_scale{ 1.0f, 1.0f, 1.0f };
    QVector3D m_pivot;
    QSSGRenderNodeTag m_tag;
    float m_opacity = 1.0f;
    int m_staticFlags = 0;
    bool m_visible = true;
    QMatrix4x4 m_sceneTransform; // Right handed
    QMatrix4x4 m_localTransform; // Right handed
    bool m_sceneTransformDirty = true;
    int m_sceneTransformConnectionCount = 0;
    int m_directionConnectionCount = 0;
    bool m_isHiddenInEditor = false;
    bool m_hasInheritedUniformScale = true;
    bool m_hasExplicitLocalTransform = false;
};


QT_END_NAMESPACE

#endif // QQUICK3DNODE_P_P_H



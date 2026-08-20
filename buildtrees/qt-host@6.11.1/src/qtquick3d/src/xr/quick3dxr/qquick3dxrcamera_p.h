// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRCAMERA_P_H
#define QQUICK3DXRCAMERA_P_H

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


#include <QtQuick3DXr/qtquick3dxrglobal.h>

#include <QObject>
#include <QtQuick3D/private/qquick3dcamera_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DXrOrigin;

class Q_QUICK3DXR_EXPORT QQuick3DXrEyeCamera : public QQuick3DCamera
{
    Q_OBJECT
    Q_PROPERTY(float leftTangent READ leftTangent WRITE setLeftTangent NOTIFY leftTangentChanged)
    Q_PROPERTY(float rightTangent READ rightTangent WRITE setRightTangent NOTIFY rightTangentChanged)
    Q_PROPERTY(float upTangent READ upTangent WRITE setUpTangent NOTIFY upTangentChanged)
    Q_PROPERTY(float downTangent READ downTangent WRITE setDownTangent NOTIFY downTangentChanged)
    Q_PROPERTY(float clipNear READ clipNear WRITE setClipNear NOTIFY clipNearChanged)
    Q_PROPERTY(float clipFar READ clipFar WRITE setClipFar NOTIFY clipFarChanged)

public:
    explicit QQuick3DXrEyeCamera(QQuick3DXrOrigin *parent = nullptr);

    float leftTangent() const;
    float rightTangent() const;
    float upTangent() const;
    float downTangent() const;
    float clipNear() const;
    float clipFar() const;

public Q_SLOTS:
    void setLeftTangent(float leftTangent);
    void setRightTangent(float rightTangent);
    void setUpTangent(float upTangent);
    void setDownTangent(float downTangent);
    void setClipNear(float clipNear);
    void setClipFar(float clipFar);
    void setProjection(const QMatrix4x4 &projection);

Q_SIGNALS:
    void leftTangentChanged(float leftTangent);
    void rightTangentChanged(float rightTangent);
    void upTangentChanged(float upTangent);
    void downTangentChanged(float downTangent);
    void clipNearChanged(float clipNear);
    void clipFarChanged(float clipFar);

public:
    QMatrix4x4 m_projection;

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    enum DirtyFlag : quint8
    {
        ProjectionDirty = 0x1, // Camera projection matrix needs to be recalculated
        ProjectionChanged = 0x2, // A camera projection matrix has changed (no recalculation needed)
        ClipChanged = 0x4, // Camera clip planes have changed
    };
    using DirtyFlagT = std::underlying_type_t<DirtyFlag>;

    void markDirty(DirtyFlag flag);
    void maybeUpdateProjection();
    float m_leftTangent = -0.017455064928218f;  // tan(-1)
    float m_rightTangent = 0.017455064928218f;  // tan(1)
    float m_upTangent = 0.017455064928218f;     // tan(1)
    float m_downTangent = -0.017455064928218f;  // tan(-1)
    float m_clipNear = 1.0f;
    float m_clipFar = 10000.0f;
    DirtyFlagT m_dirtyFlags = 0;
};

class Q_QUICK3DXR_EXPORT QQuick3DXrCamera : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(float clipNear READ clipNear WRITE setClipNear NOTIFY clipNearChanged FINAL)
    Q_PROPERTY(float clipFar READ clipFar WRITE setClipFar NOTIFY clipFarChanged FINAL)
    QML_NAMED_ELEMENT(XrCamera)
    QML_ADDED_IN_VERSION(6, 8)

public:
    explicit QQuick3DXrCamera(QQuick3DXrOrigin *parent = nullptr);
    ~QQuick3DXrCamera();
    float clipNear() const;
    float clipFar() const;

public Q_SLOTS:
    void setClipNear(float clipNear);
    void setClipFar(float clipFar);

Q_SIGNALS:
    void clipNearChanged(float clipNear);
    void clipFarChanged(float clipFar);

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;

private:
    void syncCameraSettings();

    float m_clipNear = 1.0f;
    float m_clipFar = 10000.0f;
};


QT_END_NAMESPACE

#endif // QQUICK3DXRCAMERA_P_H

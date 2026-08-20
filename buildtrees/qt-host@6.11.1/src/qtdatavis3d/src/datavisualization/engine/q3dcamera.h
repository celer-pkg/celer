// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_Q3DCAMERA_H
#define QTDATAVIS3D_Q3DCAMERA_H

#include <QtDataVisualization/q3dobject.h>

QT_BEGIN_NAMESPACE

class Q3DCameraPrivate;

class Q_DATAVISUALIZATION_EXPORT Q3DCamera : public Q3DObject
{
    Q_OBJECT
    Q_PROPERTY(float xRotation READ xRotation WRITE setXRotation NOTIFY xRotationChanged)
    Q_PROPERTY(float yRotation READ yRotation WRITE setYRotation NOTIFY yRotationChanged)
    Q_PROPERTY(float zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(CameraPreset cameraPreset READ cameraPreset WRITE setCameraPreset NOTIFY cameraPresetChanged)
    Q_PROPERTY(bool wrapXRotation READ wrapXRotation WRITE setWrapXRotation NOTIFY wrapXRotationChanged)
    Q_PROPERTY(bool wrapYRotation READ wrapYRotation WRITE setWrapYRotation NOTIFY wrapYRotationChanged)
    Q_PROPERTY(QVector3D target READ target WRITE setTarget NOTIFY targetChanged REVISION(1, 2))
    Q_PROPERTY(float minZoomLevel READ minZoomLevel WRITE setMinZoomLevel NOTIFY minZoomLevelChanged REVISION(1, 2))
    Q_PROPERTY(float maxZoomLevel READ maxZoomLevel WRITE setMaxZoomLevel NOTIFY maxZoomLevelChanged REVISION(1, 2))

public:
    enum CameraPreset {
        CameraPresetNone = -1,
        CameraPresetFrontLow = 0,
        CameraPresetFront,
        CameraPresetFrontHigh,
        CameraPresetLeftLow,
        CameraPresetLeft,
        CameraPresetLeftHigh,
        CameraPresetRightLow,
        CameraPresetRight,
        CameraPresetRightHigh,
        CameraPresetBehindLow,
        CameraPresetBehind,
        CameraPresetBehindHigh,
        CameraPresetIsometricLeft,
        CameraPresetIsometricLeftHigh,
        CameraPresetIsometricRight,
        CameraPresetIsometricRightHigh,
        CameraPresetDirectlyAbove,
        CameraPresetDirectlyAboveCW45,
        CameraPresetDirectlyAboveCCW45,
        CameraPresetFrontBelow,
        CameraPresetLeftBelow,
        CameraPresetRightBelow,
        CameraPresetBehindBelow,
        CameraPresetDirectlyBelow
    };
    Q_ENUM(CameraPreset)

    explicit Q3DCamera(QObject *parent = nullptr);
    virtual ~Q3DCamera();

    float xRotation() const;
    void setXRotation(float rotation);
    float yRotation() const;
    void setYRotation(float rotation);

    bool wrapXRotation() const;
    void setWrapXRotation(bool isEnabled);

    bool wrapYRotation() const;
    void setWrapYRotation(bool isEnabled);

    void copyValuesFrom(const Q3DObject &source) override;

    CameraPreset cameraPreset() const;
    void setCameraPreset(CameraPreset preset);

    float zoomLevel() const;
    void setZoomLevel(float zoomLevel);
    float minZoomLevel() const;
    void setMinZoomLevel(float zoomLevel);
    float maxZoomLevel() const;
    void setMaxZoomLevel(float zoomLevel);

    void setCameraPosition(float horizontal, float vertical, float zoom = 100.0f);

    QVector3D target() const;
    void setTarget(const QVector3D &target);

Q_SIGNALS:
    void xRotationChanged(float rotation);
    void yRotationChanged(float rotation);
    void zoomLevelChanged(float zoomLevel);
    void cameraPresetChanged(Q3DCamera::CameraPreset preset);
    void wrapXRotationChanged(bool isEnabled);
    void wrapYRotationChanged(bool isEnabled);
    Q_REVISION(1, 2) void targetChanged(const QVector3D &target);
    Q_REVISION(1, 2) void minZoomLevelChanged(float zoomLevel);
    Q_REVISION(1, 2) void maxZoomLevelChanged(float zoomLevel);

private:
    QScopedPointer<Q3DCameraPrivate> d_ptr;

    Q_DISABLE_COPY(Q3DCamera)

    friend class Q3DCameraPrivate;
    friend class Q3DScenePrivate;
    friend class Abstract3DRenderer;
    friend class Bars3DRenderer;
    friend class Surface3DRenderer;
    friend class Scatter3DRenderer;
    friend class SelectionPointer;
    friend class Q3DInputHandler;
    friend class QTouch3DInputHandlerPrivate;
};

QT_END_NAMESPACE

#endif

// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGPERSPECTIVECAMERA_H
#define QSSGPERSPECTIVECAMERA_H

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

#include <QtQuick3D/private/qquick3dcamera_p.h>

QT_BEGIN_NAMESPACE

class QSSGRenderCamera;
class Q_QUICK3D_EXPORT QQuick3DPerspectiveCamera : public QQuick3DCamera
{
    Q_OBJECT
    Q_PROPERTY(float clipNear READ clipNear WRITE setClipNear NOTIFY clipNearChanged)
    Q_PROPERTY(float clipFar READ clipFar WRITE setClipFar NOTIFY clipFarChanged)
    Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fieldOfViewChanged)
    Q_PROPERTY(FieldOfViewOrientation fieldOfViewOrientation READ fieldOfViewOrientation WRITE setFieldOfViewOrientation NOTIFY fieldOfViewOrientationChanged)

    QML_NAMED_ELEMENT(PerspectiveCamera)

public:
    enum FieldOfViewOrientation {
        Vertical,
        Horizontal
    };
    Q_ENUM(FieldOfViewOrientation)

    explicit QQuick3DPerspectiveCamera(QQuick3DNode *parent = nullptr);

    float clipNear() const;
    float clipFar() const;
    float fieldOfView() const;
    FieldOfViewOrientation fieldOfViewOrientation() const;

public Q_SLOTS:
    void setClipNear(float clipNear);
    void setClipFar(float clipFar);
    void setFieldOfView(float fieldOfView);
    void setFieldOfViewOrientation(QQuick3DPerspectiveCamera::FieldOfViewOrientation fieldOfViewOrientation);

Q_SIGNALS:
    void clipNearChanged();
    void clipFarChanged();
    void fieldOfViewChanged();
    void fieldOfViewOrientationChanged();

protected:
    explicit QQuick3DPerspectiveCamera(QQuick3DNodePrivate &dd, QQuick3DNode *parent = nullptr);
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    float m_clipNear = 10.0f;
    float m_clipFar = 10000.0f;
    float m_fieldOfView = 60.0f;
    FieldOfViewOrientation m_fieldOfViewOrientation = Vertical;
};

QT_END_NAMESPACE

#endif // QSSGPERSPECTIVECAMERA_H

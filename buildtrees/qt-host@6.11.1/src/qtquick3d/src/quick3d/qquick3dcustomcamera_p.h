// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGCUSTOMCAMERA_H
#define QSSGCUSTOMCAMERA_H

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
class Q_QUICK3D_EXPORT QQuick3DCustomCamera : public QQuick3DCamera
{
    Q_OBJECT
    Q_PROPERTY(QMatrix4x4 projection READ projection WRITE setProjection NOTIFY projectionChanged)

    QML_NAMED_ELEMENT(CustomCamera)

public:
    explicit QQuick3DCustomCamera(QQuick3DNode *parent = nullptr);

    QMatrix4x4 projection() const;

public Q_SLOTS:
    void setProjection(const QMatrix4x4 &projection);

Q_SIGNALS:
    void projectionChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    QMatrix4x4 m_projection;
};

QT_END_NAMESPACE

#endif // QSSGCUSTOMCAMERA_H

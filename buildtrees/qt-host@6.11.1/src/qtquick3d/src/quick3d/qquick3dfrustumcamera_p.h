// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGFRUSTUMCAMERA_H
#define QSSGFRUSTUMCAMERA_H

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

#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>

QT_BEGIN_NAMESPACE

class QSSGRenderCamera;
class Q_QUICK3D_EXPORT QQuick3DFrustumCamera : public QQuick3DPerspectiveCamera
{
    Q_OBJECT
    Q_PROPERTY(float top READ top WRITE setTop NOTIFY topChanged)
    Q_PROPERTY(float bottom READ bottom WRITE setBottom NOTIFY bottomChanged)
    Q_PROPERTY(float right READ right WRITE setRight NOTIFY rightChanged)
    Q_PROPERTY(float left READ left WRITE setLeft NOTIFY leftChanged)

    QML_NAMED_ELEMENT(FrustumCamera)

public:
    explicit QQuick3DFrustumCamera(QQuick3DNode *parent = nullptr);

    float top() const;
    float bottom() const;
    float right() const;
    float left() const;

public Q_SLOTS:
    void setTop(float top);
    void setBottom(float bottom);
    void setRight(float right);
    void setLeft(float left);

Q_SIGNALS:
    void topChanged();
    void bottomChanged();
    void rightChanged();
    void leftChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    float m_top = 0.0f;
    float m_bottom = 0.0f;
    float m_right = 0.0f;
    float m_left = 0.0f;
};

QT_END_NAMESPACE

#endif // QSSGFRUSTUMCAMERA_H

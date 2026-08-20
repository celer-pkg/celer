// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QPLANESHAPE_H
#define QPLANESHAPE_H

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
#include <QtQuick3DPhysics/private/qabstractcollisionshape_p.h>
#include <QtCore/QObject>
#include <QtGui/QVector3D>
#include <QtQml/QQmlEngine>

namespace physx {
class PxPlaneGeometry;
}

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QPlaneShape : public QAbstractCollisionShape
{
    Q_OBJECT

    QML_NAMED_ELEMENT(PlaneShape)
public:
    QPlaneShape();
    ~QPlaneShape();

    physx::PxGeometry *getPhysXGeometry() override;
    bool isStaticShape() const override { return true; }

private:
    void updatePhysXGeometry();

    physx::PxPlaneGeometry *m_planeGeometry = nullptr;
};

QT_END_NAMESPACE

#endif // QPLANESHAPE_H

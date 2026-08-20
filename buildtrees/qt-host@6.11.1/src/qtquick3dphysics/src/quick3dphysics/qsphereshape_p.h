// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SPHERESHAPE_H
#define SPHERESHAPE_H

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

#include <QtQuick3DPhysics/private/qabstractcollisionshape_p.h>
#include <QtQml/QQmlEngine>

namespace physx {
class PxSphereGeometry;
};

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QSphereShape : public QAbstractCollisionShape
{
    Q_OBJECT
    Q_PROPERTY(float diameter READ diameter WRITE setDiameter NOTIFY diameterChanged)
    QML_NAMED_ELEMENT(SphereShape)
public:
    QSphereShape();
    ~QSphereShape();

    float diameter() const;

    physx::PxGeometry *getPhysXGeometry() override;
    bool isStaticShape() const override { return false; }

public Q_SLOTS:
    void setDiameter(float diameter);

Q_SIGNALS:
    void diameterChanged(float diameter);

private:
    void updatePhysXGeometry();
    physx::PxSphereGeometry *m_physXGeometry = nullptr;
    float m_diameter = 100.0f;
};

QT_END_NAMESPACE

#endif // SPHERESHAPE_H

// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CAPSULESHAPE_H
#define CAPSULESHAPE_H

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
#include <QtQml/QQmlEngine>

namespace physx {
class PxCapsuleGeometry;
}

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QCapsuleShape : public QAbstractCollisionShape
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CapsuleShape)
    Q_PROPERTY(float diameter READ diameter WRITE setDiameter NOTIFY diameterChanged)
    Q_PROPERTY(float height READ height WRITE setHeight NOTIFY heightChanged)
public:
    QCapsuleShape();
    ~QCapsuleShape();

    float diameter() const;
    void setDiameter(float newDiameter);

    float height() const;
    void setHeight(float newHeight);

    physx::PxGeometry *getPhysXGeometry() override;
    bool isStaticShape() const override { return false; }

Q_SIGNALS:
    void diameterChanged();
    void heightChanged();

private:
    void updatePhysXGeometry();
    physx::PxCapsuleGeometry *m_physXGeometry = nullptr;
    float m_diameter = 100.0f;
    float m_height = 100.0f;
};

QT_END_NAMESPACE

#endif // CAPSULESHAPE_H

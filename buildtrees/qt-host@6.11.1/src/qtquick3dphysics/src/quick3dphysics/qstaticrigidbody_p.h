// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef STATICRIGIDBODY_H
#define STATICRIGIDBODY_H

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

#include <QtQuick3DPhysics/private/qabstractphysicsbody_p.h>
#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QStaticRigidBody : public QAbstractPhysicsBody
{
    Q_OBJECT
    QML_NAMED_ELEMENT(StaticRigidBody)
public:
    QStaticRigidBody();
    QAbstractPhysXNode *createPhysXBackend() final;
};

QT_END_NAMESPACE

#endif // STATICRIGIDBODY_H

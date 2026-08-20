// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TRIGGERBODY_H
#define TRIGGERBODY_H

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
#include "qabstractphysicsnode_p.h"
#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QTriggerBody : public QAbstractPhysicsNode
{
    Q_OBJECT
    Q_PROPERTY(int collisionCount READ collisionCount NOTIFY collisionCountChanged)
    QML_NAMED_ELEMENT(TriggerBody)
public:
    QTriggerBody();

    void registerCollision(QAbstractPhysicsNode *collision);
    void deregisterCollision(QAbstractPhysicsNode *collision);

    int collisionCount() const;
    QAbstractPhysXNode *createPhysXBackend() final;

Q_SIGNALS:
    void bodyEntered(QAbstractPhysicsNode *body);
    void bodyExited(QAbstractPhysicsNode *body);
    void collisionCountChanged();

private:
    QSet<QAbstractPhysicsNode *> m_collisions;
};

QT_END_NAMESPACE

#endif // TRIGGERBODY_H

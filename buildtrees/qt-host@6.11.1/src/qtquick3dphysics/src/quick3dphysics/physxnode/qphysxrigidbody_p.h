// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PHYSXRIGIDBODY_H
#define PHYSXRIGIDBODY_H

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

#include "physxnode/qphysxactorbody_p.h"

#include "qtconfigmacros.h"

QT_BEGIN_NAMESPACE

class QAbstractPhysicsBody;

class QPhysXRigidBody : public QPhysXActorBody
{
public:
    QPhysXRigidBody(QAbstractPhysicsBody *frontEnd);
    void createMaterial(QPhysXWorld *physX) override;
};

QT_END_NAMESPACE

#endif

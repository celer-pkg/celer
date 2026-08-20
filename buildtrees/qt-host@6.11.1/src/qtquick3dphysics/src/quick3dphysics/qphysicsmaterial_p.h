// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QPHYSICSMATERIAL_H
#define QPHYSICSMATERIAL_H

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
#include <QtQml/QQmlEngine>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPHYSICS_EXPORT QPhysicsMaterial : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float staticFriction READ staticFriction WRITE setStaticFriction NOTIFY
                       staticFrictionChanged)
    Q_PROPERTY(float dynamicFriction READ dynamicFriction WRITE setDynamicFriction NOTIFY
                       dynamicFrictionChanged)
    Q_PROPERTY(float restitution READ restitution WRITE setRestitution NOTIFY restitutionChanged)
    QML_NAMED_ELEMENT(PhysicsMaterial)
public:
    explicit QPhysicsMaterial(QObject *parent = nullptr);

    float staticFriction() const;
    void setStaticFriction(float staticFriction);

    float dynamicFriction() const;
    void setDynamicFriction(float dynamicFriction);

    float restitution() const;
    void setRestitution(float restitution);

    static constexpr float defaultStaticFriction = 0.5f;
    static constexpr float defaultDynamicFriction = 0.5f;
    static constexpr float defaultRestitution = 0.5f;

Q_SIGNALS:
    void staticFrictionChanged();
    void dynamicFrictionChanged();
    void restitutionChanged();

private:
    float m_staticFriction = defaultStaticFriction;
    float m_dynamicFriction = defaultDynamicFriction;
    float m_restitution = defaultRestitution;
};

QT_END_NAMESPACE

#endif // QPHYSICSMATERIAL_H

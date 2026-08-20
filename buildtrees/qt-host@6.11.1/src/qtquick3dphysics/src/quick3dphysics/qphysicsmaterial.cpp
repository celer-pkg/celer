// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qphysicsmaterial_p.h"

#include <foundation/PxSimpleTypes.h>

static float clamp(float value, float min, float max)
{
    return std::max(std::min(value, max), min);
}

QT_BEGIN_NAMESPACE

/*!
    \qmltype PhysicsMaterial
    \inqmlmodule QtQuick3D.Physics
    \since 6.4
    \brief Defines the physics material of a body.

    The PhysicsMaterial type determines how objects interact when they touch.

    Friction uses the Coulomb friction model, which is based around
    the concepts of 2 coefficients: the static friction coefficient and the dynamic friction
    coefficient (sometimes called kinetic friction). Friction resists relative lateral motion of two
    solid surfaces in contact. These two coefficients define a relationship between the normal force
    exerted by each surface on the other and the amount of friction force that is applied to resist
    lateral motion. While most real-world materials have friction coefficients between \c{0} and
    \c{1}, values above \c{1} are not uncommon. The properties accept any real number greater or
    equal to \c{0}.

    Restitution determines how objects bounce when they collide.
*/

/*!
    \qmlproperty real PhysicsMaterial::staticFriction
    This property defines the amount of friction that is applied between surfaces that are not
    moving lateral to each-other.

    Default value: \c 0.5

    Range: \c{[0, inf]}
*/

/*!
    \qmlproperty real PhysicsMaterial::dynamicFriction
    This property defines the amount of friction applied between surfaces that are moving relative
    to each-other.

    Default value: \c 0.5

    Range: \c{[0, inf]}
*/

/*!
    \qmlproperty real PhysicsMaterial::restitution
    This property defines the coefficient of restitution, or how bouncy the material is.
    The coefficient of restitution of two
    colliding objects is a fractional value representing the ratio of speeds after and before an
    impact, taken along the line of impact. A coefficient of restitution of 1 is said to collide
    elastically, while a coefficient of restitution < 1 is said to be inelastic.

    Default value: \c 0.5

    Range: \c{[0, 1]}
*/

QPhysicsMaterial::QPhysicsMaterial(QObject *parent) : QObject(parent) { }

float QPhysicsMaterial::staticFriction() const
{
    return m_staticFriction;
}

void QPhysicsMaterial::setStaticFriction(float staticFriction)
{
    staticFriction = clamp(staticFriction, 0.f, PX_MAX_F32);

    if (qFuzzyCompare(m_staticFriction, staticFriction))
        return;
    m_staticFriction = staticFriction;
    emit staticFrictionChanged();
}

float QPhysicsMaterial::dynamicFriction() const
{
    return m_dynamicFriction;
}

void QPhysicsMaterial::setDynamicFriction(float dynamicFriction)
{
    dynamicFriction = clamp(dynamicFriction, 0.f, PX_MAX_F32);

    if (qFuzzyCompare(m_dynamicFriction, dynamicFriction))
        return;
    m_dynamicFriction = dynamicFriction;
    emit dynamicFrictionChanged();
}

float QPhysicsMaterial::restitution() const
{
    return m_restitution;
}

void QPhysicsMaterial::setRestitution(float restitution)
{
    restitution = clamp(restitution, 0.f, 1.f);

    if (qFuzzyCompare(m_restitution, restitution))
        return;
    m_restitution = restitution;
    emit restitutionChanged();
}

QT_END_NAMESPACE

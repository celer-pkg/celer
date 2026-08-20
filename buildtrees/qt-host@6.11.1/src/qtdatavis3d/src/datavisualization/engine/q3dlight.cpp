// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "q3dlight_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \class Q3DLight
 * \inmodule QtDataVisualization
 * \brief Representation of a light source in 3D space.
 * \since QtDataVisualization 1.0
 *
 * Q3DLight represents a monochrome light source in 3D space.
 *
 * \note Default light has isAutoPosition() \c true.
 */

/*!
 * \qmltype Light3D
 * \inqmlmodule QtDataVisualization
 * \since QtDataVisualization 1.0
 * \ingroup datavisualization_qml
 * \nativetype Q3DLight
 * \brief Representation of a light source in 3D space.
 *
 * Light3D represents a monochrome light source in 3D space.
 *
 * \note Default light has autoPosition \c true.
 */

/*!
 * \qmlproperty bool Light3D::autoPosition
 * \since QtDataVisualization 1.3
 * Defines whether the light position follows the camera automatically.
 * \note Has no effect if shadows are enabled. Remember to disable shadows before setting light's
 * position, or it will be overwritten by automatic positioning if this
 * property is \c false.
 */

/*!
 * Constructs a new 3D light located at origin. An optional \a parent parameter can be given
 * and is then passed to QObject constructor.
 */
Q3DLight::Q3DLight(QObject *parent) :
    Q3DObject(parent),
    d_ptr(new Q3DLightPrivate(this))
{
}

/*!
 * Destroys the light object.
 */
Q3DLight::~Q3DLight()
{
}

/*!
 * \property Q3DLight::autoPosition
 * \since QtDataVisualization 5.9
 * \brief Whether the light position follows the camera automatically.
 * \note Has no effect if shadows are enabled. Remember to disable shadows before setting light's
 * position, or it will be overwritten by automatic positioning if
 * \c isAutoPosition() is \c false.
 */
void Q3DLight::setAutoPosition(bool enabled)
{
    if (enabled != d_ptr->m_automaticLight) {
        d_ptr->m_automaticLight = enabled;
        setDirty(true);
        emit autoPositionChanged(enabled);
    }
}

bool Q3DLight::isAutoPosition()
{
    return d_ptr->m_automaticLight;
}

Q3DLightPrivate::Q3DLightPrivate(Q3DLight *q) :
    q_ptr(q),
    m_automaticLight(false)
{
}

Q3DLightPrivate::~Q3DLightPrivate()
{
}

void Q3DLightPrivate::sync(Q3DLight &other)
{
    if (q_ptr->isDirty()) {
        other.setPosition(q_ptr->position());
        other.setAutoPosition(q_ptr->isAutoPosition());
        q_ptr->setDirty(false);
    }
}

QT_END_NAMESPACE

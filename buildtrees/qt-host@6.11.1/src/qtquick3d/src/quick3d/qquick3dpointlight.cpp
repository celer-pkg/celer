// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dpointlight_p.h"
#include "qquick3dobject_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>

#include "qquick3dnode_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype PointLight
    \inherits Light
    \inqmlmodule QtQuick3D
    \brief Defines a point light in the scene.

    The point light can be described as a sphere, emitting light with equal strength in all
    directions from the center of the light. This is similar to the way a light bulb emits light.

    Rotating or scaling a point light does not have any effect. Moving a point light will change
    the position from where the light is emitted.

    By default, a point light intensity diminishes according to inverse-square-law. However, the fade-off
    (and range) can be controlled with the \l {constantFade}, \l {linearFade}, and
    \l quadraticFade properties. Light attenuation is calculated using the formula:
    \l {constantFade} + \c distance * (\l {linearFade} * 0.01) + \c distance^2 * (\l {quadraticFade} * 0.0001)

    \section2 A simple example: shading a sphere in three different ways

    Take a scene containing a sphere in front of a scaled up rectangle in the
    background. The default base color of the PrincipledMaterial of the
    rectangle is white.

    Without any lights and disabling light-related shading for the two meshes,
    we get the following:

    \qml
    import QtQuick
    import QtQuick3D
    View3D {
        anchors.fill: parent

        PerspectiveCamera { z: 600 }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(10, 10, 10)
            z: -100
            materials: PrincipledMaterial {
                lighting: PrincipledMaterial.NoLighting
            }
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            materials: PrincipledMaterial {
                lighting: PrincipledMaterial.NoLighting
                baseColor: "#40c060"
                roughness: 0.1
            }
        }
    }
    \endqml

    \image pointlight-1.png
           {Green circle with white background}

    Adding a directional light, emitting down the Z axis by default, leads to the following:

    \qml
    import QtQuick
    import QtQuick3D
    View3D {
        anchors.fill: parent

        PerspectiveCamera { z: 600 }

        DirectionalLight { }

        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(10, 10, 10)
            z: -100
            materials: PrincipledMaterial { }
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(2, 2, 2)
            materials: PrincipledMaterial {
                baseColor: "#40c060"
                roughness: 0.1
            }
        }
    }
    \endqml

    \image pointlight-2.png
           {Green circle with black edge}

    What if we now replace DirectionalLight with:

    \qml
        PointLight {
            z: 300
        }
    \endqml

    The white colored PointLight here is moved on the Z axis so that it is
    halfway between the camera and the center of the scene. Unlike
    DirectionalLight, the rotation of the PointLight does not matter, whereas
    its position is significant. The diminishing intensity is visible here,
    especially on the rectangle mesh in the background.

    \image pointlight-3.png
           {Green gradient circle with black edge and dark gray gradient
           background showing point light attenuation}

    For more usage examples, see \l{Qt Quick 3D - Lights Example}.

    \sa DirectionalLight, SpotLight, {Shadow Mapping}
*/

/*!
    \qmlproperty real PointLight::constantFade

    This property is constant factor of the attenuation term of the light.
    The default value is 1.0.
 */

/*!
    \qmlproperty real PointLight::linearFade

    This property increases the rate at which the lighting effect dims the light
    in proportion to the distance to the light. The default value is \c 0.0, meaning the light doesn't
    have linear fade. The value used here is multiplied by \c 0.01 before being used to
    calculate light attenuation.
*/

/*!
    \qmlproperty real PointLight::quadraticFade

    This property increases the rate at which the lighting effect dims the light
    in proportion to the inverse square law. The default value is 1.0 meaning the point light
    fade exactly follows the inverse square law i.e. when distance to an object doubles the
    light intensity decreases to 1/4th. The value used here is multiplied by \c 0.0001 before
    being used to calculate light attenuation.
*/

QQuick3DPointLight::QQuick3DPointLight(QQuick3DNode *parent)
    : QQuick3DAbstractLight(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::PointLight)), parent) {}

float QQuick3DPointLight::constantFade() const
{
    return m_constantFade;
}

float QQuick3DPointLight::linearFade() const
{
    return m_linearFade;
}

float QQuick3DPointLight::quadraticFade() const
{
    return m_quadraticFade;
}

void QQuick3DPointLight::setConstantFade(float constantFade)
{
    if (qFuzzyCompare(m_constantFade, constantFade))
        return;

    m_constantFade = constantFade;
    m_dirtyFlags.setFlag(DirtyFlag::FadeDirty);
    emit constantFadeChanged();
    update();
}

void QQuick3DPointLight::setLinearFade(float linearFade)
{
    if (qFuzzyCompare(m_linearFade, linearFade))
        return;

    m_linearFade = linearFade;
    m_dirtyFlags.setFlag(DirtyFlag::FadeDirty);
    emit linearFadeChanged();
    update();
}

void QQuick3DPointLight::setQuadraticFade(float quadraticFade)
{
    if (qFuzzyCompare(m_quadraticFade, quadraticFade))
        return;

    m_quadraticFade = quadraticFade;
    m_dirtyFlags.setFlag(DirtyFlag::FadeDirty);
    emit quadraticFadeChanged();
    update();
}

QSSGRenderGraphObject *QQuick3DPointLight::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderLight(QSSGRenderLight::Type::PointLight);
    }

    QQuick3DAbstractLight::updateSpatialNode(node); // Marks the light node dirty if m_dirtyFlags != 0

    QSSGRenderLight *light = static_cast<QSSGRenderLight *>(node);

    if (m_dirtyFlags.testFlag(DirtyFlag::FadeDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::FadeDirty, false);
        light->m_constantFade = m_constantFade;
        light->m_linearFade = m_linearFade;
        light->m_quadraticFade = m_quadraticFade;
    }

    return node;
}

QT_END_NAMESPACE

// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dspotlight_p.h"
#include "qquick3dobject_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>

#include "qquick3dnode_p_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype SpotLight
    \inherits Light
    \inqmlmodule QtQuick3D
    \brief Defines a spot light in the scene.
    \since 5.15

    The spot light emits light towards one direction in a cone shape, which is defined by the
    \l {coneAngle} property. The light intensity diminishes when approaching the \l {coneAngle}.
    The angle at which the light intensity starts to diminish is defined by \l {innerConeAngle}.
    Both angles are defined in degrees.

    Inside the \l {innerConeAngle}, the spot light behaves similarly to the point light.
    There the light intensity diminishes according to inverse-square-law. However, the fade-off
    (and range) can be controlled with the \l {constantFade}, \l {linearFade}, and
    \l quadraticFade properties. Light attenuation is calculated using the formula:
    \l {constantFade} + \c distance * (\l {linearFade} * 0.01) + \c distance * (\l {quadraticFade} * 0.0001)^2

    Let's look at a simple example. Here a SpotLight is placed at 300 on the Z
    axis, so halfway between the camera and the scene center. By default the
    light is emitting in the direction of the Z axis. The \l {Light::}{brightness} is
    increased to 10 to make it look more like a typical spot light.

    \qml
    import QtQuick
    import QtQuick3D
    View3D {
        anchors.fill: parent

        PerspectiveCamera { z: 600 }

        SpotLight {
            z: 300
            brightness: 10
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }

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

    \image spotlight-1.png {Green sphere with spotlight cone angle visualization}

    Rotations happens similarly to \l DirectionalLight. Here we want to light to
    emit more to the right, so we rotate around the Y axis by -20 degrees. The
    cone is reduced by setting coneAngle to 30 instead of the default 40.  We
    also make the intensity start diminish earlier, by changing innerConeAngle
    to 10.

    \qml
    SpotLight {
        z: 300
        brightness: 10
        ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        eulerRotation.y: -20
        coneAngle: 30
        innerConeAngle: 10
    }
    \endqml

    \image spotlight-2.png {Green sphere with spotlight inner and outer cone angles}

    For further usage examples, see \l{Qt Quick 3D - Lights Example}.

    \sa DirectionalLight, PointLight, {Shadow Mapping}
*/

/*!
    \qmlproperty real SpotLight::constantFade

    This property is constant factor of the attenuation term of the light.
    The default value is 1.0.
 */

/*!
    \qmlproperty real SpotLight::linearFade

    This property increases the rate at which the lighting effect dims the light
    in proportion to the distance to the light. The default value is \c 0.0, which means the light
    doesn't have linear fade. The value used here is multiplied by \c 0.01 before being used to
    calculate light attenuation.
*/

/*!
    \qmlproperty real SpotLight::quadraticFade

    This property increases the rate at which the lighting effect dims the light
    in proportion to the inverse square law. The default value is 1.0, which means the spot light
    fade exactly follows the inverse square law, i.e. when distance to an object doubles the
    light intensity decreases to 1/4th. The value used here is multiplied by \c 0.0001 before
    being used to calculate light attenuation.
*/

/*!
    \qmlproperty real SpotLight::coneAngle

    This property defines the cut-off angle (from edge to edge) beyond which the light doesn't affect the scene.
    Defined in degrees between \c{0} and \c{180}. The default value is \c{40}.

    \note When the cone angle approaches \c{180} degrees the shadow quality will start to deteriorate. A value
    under \c{170} is therefore recommended.
*/

/*!
    \qmlproperty real SpotLight::innerConeAngle

    This property defines the angle (from edge to edge) at which the light intensity starts to gradually diminish
    as it approaches \l {coneAngle}. Defined in degrees between 0 and 180. If the value is set
    larger than \l {coneAngle}, it'll behave as if it had the same value as \l {coneAngle}.
    The default value is 30.
*/

QQuick3DSpotLight::QQuick3DSpotLight(QQuick3DNode *parent)
    : QQuick3DAbstractLight(*(new QQuick3DNodePrivate(QQuick3DNodePrivate::Type::SpotLight)), parent) {}

float QQuick3DSpotLight::constantFade() const
{
    return m_constantFade;
}

float QQuick3DSpotLight::linearFade() const
{
    return m_linearFade;
}

float QQuick3DSpotLight::quadraticFade() const
{
    return m_quadraticFade;
}

float QQuick3DSpotLight::coneAngle() const
{
    return m_coneAngle;
}

float QQuick3DSpotLight::innerConeAngle() const
{
    return m_innerConeAngle;
}

void QQuick3DSpotLight::setConstantFade(float constantFade)
{
    if (qFuzzyCompare(m_constantFade, constantFade))
        return;

    m_constantFade = constantFade;
    m_dirtyFlags.setFlag(DirtyFlag::FadeDirty);
    emit constantFadeChanged();
    update();
}

void QQuick3DSpotLight::setLinearFade(float linearFade)
{
    if (qFuzzyCompare(m_linearFade, linearFade))
        return;

    m_linearFade = linearFade;
    m_dirtyFlags.setFlag(DirtyFlag::FadeDirty);
    emit linearFadeChanged();
    update();
}

void QQuick3DSpotLight::setQuadraticFade(float quadraticFade)
{
    if (qFuzzyCompare(m_quadraticFade, quadraticFade))
        return;

    m_quadraticFade = quadraticFade;
    m_dirtyFlags.setFlag(DirtyFlag::FadeDirty);
    emit quadraticFadeChanged();
    update();
}

void QQuick3DSpotLight::setConeAngle(float coneAngle)
{
    if (coneAngle < 0.f)
        coneAngle = 0.f;
    else if (coneAngle > 180.f)
        coneAngle = 180.f;

    if (qFuzzyCompare(m_coneAngle, coneAngle))
        return;

    m_coneAngle = coneAngle;
    m_dirtyFlags.setFlag(DirtyFlag::AreaDirty);
    emit coneAngleChanged();
    update();
}

void QQuick3DSpotLight::setInnerConeAngle(float innerConeAngle)
{
    if (innerConeAngle < 0.f)
        innerConeAngle = 0.f;
    else if (innerConeAngle > 180.f)
        innerConeAngle = 180.f;

    if (qFuzzyCompare(m_innerConeAngle, innerConeAngle))
        return;

    m_innerConeAngle = innerConeAngle;
    m_dirtyFlags.setFlag(DirtyFlag::AreaDirty);
    emit innerConeAngleChanged();
    update();
}

QSSGRenderGraphObject *QQuick3DSpotLight::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderLight(QSSGRenderLight::Type::SpotLight);
    }

    QQuick3DAbstractLight::updateSpatialNode(node); // Marks the light node dirty if m_dirtyFlags != 0

    QSSGRenderLight *light = static_cast<QSSGRenderLight *>(node);

    if (m_dirtyFlags.testFlag(DirtyFlag::FadeDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::FadeDirty, false);
        light->m_constantFade = m_constantFade;
        light->m_linearFade = m_linearFade;
        light->m_quadraticFade = m_quadraticFade;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::AreaDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::AreaDirty, false);
        light->m_coneAngle = qBound(0.0f, m_coneAngle * 0.5, 90.0f);
        light->m_innerConeAngle = qBound(0.0f, m_innerConeAngle * 0.5, 90.0f);
    }

    return node;
}

QT_END_NAMESPACE

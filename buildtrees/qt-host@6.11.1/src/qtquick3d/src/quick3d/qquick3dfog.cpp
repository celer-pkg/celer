// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dfog_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Fog
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Specifies fog settings for a scene.
    \since 6.5

    When the \l{SceneEnvironment::fog}{fog} property of a \l
    SceneEnvironment is set to a valid Fog object, the properties are used to
    configure the rendering of fog.

    \image fog.jpg
           {Scene with fog effect}

    The simple fog provided by this type is implemented by the materials. It is
    not a post-processing effect, meaning it does not involve additional render
    passes processing the texture with the output of the \l View3D, but is
    rather implemented in the fragment shader for each renderable object
    (submesh of \l Model) with a \l PrincipledMaterial or shaded \l
    CustomMaterial.

    Fog is configured by a number of properties:

    \list

    \li General settings: \l color and \l density

    \li Depth fog settings: \l depthEnabled, \l depthNear, \l depthFar, \l depthCurve

    \li Height fog settings: \l heightEnabled, \l leastIntenseY, \l mostIntenseY, \l heightCurve

    \li Color transmission settings: \l transmitEnabled, \l transmitCurve

    \endlist

    For example, the following snippet enables depth (but not height) fog using
    the default fog parameters:

    \qml
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: theFog.color
            fog: Fog {
                id: theFog
                enabled: true
                depthEnabled: true
            }
        }
    \endqml

    Instead of defining the Fog object inline, it is also possible to reference
    a Fog object by \c id. And since \l ExtendedSceneEnvironment inherits
    everything from its parent type \l SceneEnvironment, fog can be used with
    \l ExtendedSceneEnvironment as well:

    \qml
        Fog {
            id: theFog
            enabled: true
            depthEnabled: true
        }
        environment: ExtendedSceneEnvironment {
            fog: theFog
        }
    \endqml

    \sa {Qt Quick 3D - Simple Fog Example}, {Qt Quick 3D - Scene Effects Example}
 */

/*!
    \qmlproperty bool Fog::enabled

    Controls whether fog is applied to the scene. The default value is false.

    Enabling depth or height fog has no effect without setting this value to
    true.

    \sa depthEnabled, heightEnabled
 */

bool QQuick3DFog::isEnabled() const
{
    return m_enabled;
}

void QQuick3DFog::setEnabled(bool newEnabled)
{
    if (m_enabled == newEnabled)
        return;

    m_enabled = newEnabled;
    emit enabledChanged();
    emit changed();
}

/*!
    \qmlproperty color Fog::color

    The color of the fog. The default value is "#8099b3"

    \image fog_color_1.jpg
           {Scene with slightly desaturated blue fog}


    The same scene with color changed to be more blueish:

    \image fog_color_2.jpg
           {Scene with blue fog}

    \sa density
 */

QColor QQuick3DFog::color() const
{
    return m_color;
}

void QQuick3DFog::setColor(const QColor &newColor)
{
    if (m_color == newColor)
        return;

    m_color = newColor;
    emit colorChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::density

    Controls the fog amount, in practice this is a multiplier in range 0-1. The
    default value is 1.0. Reducing the value decreases the strength of the fog
    effect. Applicable only when depthEnabled is set to true.

    The on-screen visual effect may be affected by a number of other settings
    from \l ExtendedSceneEnvironment, such as tonemapping or glow and bloom.
    The same density value may give different results depending on what other
    effects are enabled, and how those are configured.

    An example scene with density set to \c{0.95}:

    \image fog_density_095.jpg
           {Scene with fog density of 95 percent}

    The same scene with density reduced to \c{0.15}:

    \image fog_density_015.jpg
           {Scene with fog density of 15 percent}

    \sa color
 */

float QQuick3DFog::density() const
{
    return m_density;
}

void QQuick3DFog::setDensity(float newDensity)
{
    if (qFuzzyCompare(m_density, newDensity))
        return;

    m_density = newDensity;
    emit densityChanged();
    emit changed();
}

/*!
    \qmlproperty bool Fog::depthEnabled

    Controls if the fog appears in the distance. The default value is false.

    \sa heightEnabled, enabled, depthNear, depthFar, depthCurve
 */

bool QQuick3DFog::isDepthEnabled() const
{
    return m_depthEnabled;
}

void QQuick3DFog::setDepthEnabled(bool newDepthEnabled)
{
    if (m_depthEnabled == newDepthEnabled)
        return;

    m_depthEnabled = newDepthEnabled;
    emit depthEnabledChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::depthNear

    Starting distance from the camera. The default value is 10.0. Applicable
    only when depthEnabled is set to true.

    As an example, take this scene, first with a higher depthNear value.

    \image fog_depthnear_higher.jpg
           {Scene with higher depthNear fog value}

    Decreasing the value of depthNear results in the fog effectively "moving
    closer" to the camera as it now starts from a smaller distance from the
    camera:

    \image fog_depthnear_lower.jpg
           {Scene with lower depthNear fog value}

    \note The scene, including the camera and the models, are expected to be set
    up accordingly, so that sensible ranges can be defined by properties such
    as depthNear and depthFar. Do not expect that fog can always be enabled on
    a scene containing assets imported as-is, without tuning the transforms
    first. For example, the example screenshots on this page with the
    \l{https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/Sponza}{Sponza}
    model are generated after manually applying an additional scale of \c{(100,
    100, 100)} on the instantiated Sponza component that was generated by the
    \c balsam tool from the glTF source asset. This then gave a sufficient Z
    range to get good looking results by tuning the depthNear and depthFar
    values.

    \sa depthFar, depthEnabled
 */

float QQuick3DFog::depthNear() const
{
    return m_depthNear;
}

void QQuick3DFog::setDepthNear(float newDepthNear)
{
    if (qFuzzyCompare(m_depthNear, newDepthNear))
        return;

    m_depthNear = newDepthNear;
    emit depthNearChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::depthFar

    Ending distance from the camera. The default value is 1000.0. Applicable
    only when depthEnabled is set to true.

    \note The scene, including the camera and the models, are expected to be set
    up accordingly, so that sensible ranges can be defined by properties such
    as depthNear and depthFar. Do not expect that fog can always be enabled on
    a scene containing assets imported as-is, without tuning the transforms
    first. For example, the example screenshots on this page with the
    \l{https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/Sponza}{Sponza}
    model are generated after manually applying an additional scale of \c{(100,
    100, 100)} on the instantiated Sponza component that was generated by the
    \c balsam tool from the glTF source asset. This then gave a sufficient Z
    range to get good looking results by tuning the depthNear and depthFar
    values.

    \sa depthNear, depthEnabled
 */

float QQuick3DFog::depthFar() const
{
    return m_depthFar;
}

void QQuick3DFog::setDepthFar(float newDepthFar)
{
    if (qFuzzyCompare(m_depthFar, newDepthFar))
        return;

    m_depthFar = newDepthFar;
    emit depthFarChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::depthCurve

    The default value is 1.0.

    Applicable only when depthEnabled is set to true.

    \sa depthEnabled
 */

float QQuick3DFog::depthCurve() const
{
    return m_depthCurve;
}

void QQuick3DFog::setDepthCurve(float newDepthCurve)
{
    if (qFuzzyCompare(m_depthCurve, newDepthCurve))
        return;

    m_depthCurve = newDepthCurve;
    emit depthCurveChanged();
    emit changed();
}

/*!
    \qmlproperty bool Fog::heightEnabled

    Controls if a height fog is enabled. The default value is false.

    \sa depthEnabled, enabled, leastIntenseY, mostIntenseY, heightCurve
 */

bool QQuick3DFog::isHeightEnabled() const
{
    return m_heightEnabled;
}

void QQuick3DFog::setHeightEnabled(bool newHeightEnabled)
{
    if (m_heightEnabled == newHeightEnabled)
        return;

    m_heightEnabled = newHeightEnabled;
    emit heightEnabledChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::leastIntenseY

    Specifies the position (Y coordinate) where the fog is the least intense.
    The default value is 10.0. Applicable only when heightEnabled is set to
    true.

    \note By default the value is larger than mostIntenseY. As long as this is
    true, the fog is rendered top to bottom. When this value is smaller than
    mostIntenseY, the fog will render bottom to top.

    \note The Y axis points upwards in Qt Quick 3D scenes.

    Pictured here is a scene with height fog enabled (no depth fog), and
    leastIntenseY set to a value so the fog is only spreading around the bottom
    of the Sponza scene.

    \image fog_height_least_y_smaller.jpg
           {Scene with lower leastIntenseY fog value}

    Increasing the value of leastIntenseY makes the fog spread higher since it
    now effectively starts at a higher Y position in the scene. (remember that
    the Y axis points upwards)

    \image fog_height_least_y_bigger.jpg
           {Scene with higher leastIntenseY fog value}

    \note As with depth fog, the scene is expected to be set up accordingly, so
    that sensible Y coordinate ranges can be defined by leastIntenseY and
    mostIntenseY. Do not expect that fog can always be enabled on a scene
    containing assets imported as-is, without tuning the transforms first. For
    example, the example screenshots on this page with the
    \l{https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/Sponza}{Sponza}
    model are generated after manually applying an additional scale of \c{(100,
    100, 100)} on the instantiated Sponza component that was generated by the
    \c balsam tool from the glTF source asset.

    \sa mostIntenseY, heightEnabled
 */

float QQuick3DFog::leastIntenseY() const
{
    return m_leastIntenseY;
}

void QQuick3DFog::setLeastIntenseY(float newLeastIntenseY)
{
    if (qFuzzyCompare(m_leastIntenseY, newLeastIntenseY))
        return;

    m_leastIntenseY = newLeastIntenseY;
    emit leastIntenseYChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::mostIntenseY

    Specifies the position (Y coordinate) where the fog is the most intense.
    The default value is 0. Applicable only when heightEnabled is set to true.

    \note By default the value is smaller than leastIntenseY. As long as this is
    true, the fog is rendered top to bottom. When this value is larger than
    leastIntenseY, the fog will render bottom to top.

    \note The Y axis points upwards in Qt Quick 3D scenes.

    \note As with depth fog, the scene is expected to be set up accordingly, so
    that sensible Y coordinate ranges can be defined by leastIntenseY and
    mostIntenseY. Do not expect that fog can always be enabled on a scene
    containing assets imported as-is, without tuning the transforms first. For
    example, the example screenshots on this page with the
    \l{https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/Sponza}{Sponza}
    model are generated after manually applying an additional scale of \c{(100,
    100, 100)} on the instantiated Sponza component that was generated by the
    \c balsam tool from the glTF source asset.

    \sa leastIntenseY, heightEnabled
 */

float QQuick3DFog::mostIntenseY() const
{
    return m_mostIntenseY;
}

void QQuick3DFog::setMostIntenseY(float newMostIntenseY)
{
    if (qFuzzyCompare(m_mostIntenseY, newMostIntenseY))
        return;

    m_mostIntenseY = newMostIntenseY;
    emit mostIntenseYChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::heightCurve

    Specifies the intensity of the height fog. The default value is 1.0.
    Applicable only when heightEnabled is set to true.

    \sa heightEnabled
 */

float QQuick3DFog::heightCurve() const
{
    return m_heightCurve;
}

void QQuick3DFog::setHeightCurve(float newHeightCurve)
{
    if (qFuzzyCompare(m_heightCurve, newHeightCurve))
        return;

    m_heightCurve = newHeightCurve;
    emit heightCurveChanged();
    emit changed();
}

/*!
    \qmlproperty bool Fog::transmitEnabled

    Controls if the fog has a light transmission effect. The default value is
    false.
 */

bool QQuick3DFog::isTransmitEnabled() const
{
    return m_transmitEnabled;
}

void QQuick3DFog::setTransmitEnabled(bool newTransmitEnabled)
{
    if (m_transmitEnabled == newTransmitEnabled)
        return;

    m_transmitEnabled = newTransmitEnabled;
    emit transmitEnabledChanged();
    emit changed();
}

/*!
    \qmlproperty real Fog::transmitCurve

    Intensity of the light transmission effect. The default value is 1.0.
    Applicable only when transmitEnabled is set to true.
 */

float QQuick3DFog::transmitCurve() const
{
    return m_transmitCurve;
}

void QQuick3DFog::setTransmitCurve(float newTransmitCurve)
{
    if (qFuzzyCompare(m_transmitCurve, newTransmitCurve))
        return;

    m_transmitCurve = newTransmitCurve;
    emit transmitCurveChanged();
    emit changed();
}

QT_END_NAMESPACE

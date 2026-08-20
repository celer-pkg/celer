// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3ddebugsettings_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype DebugSettings
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Used to configure debug settings.

    The renderer can be configured to output many different views to facilitate
    debugging. This component is used to configure these debug views.

    In addition to programatic control, properties such as \l materialOverride
    and \l wireframeEnabled can also be controlled interactively via the \l
    DebugView item if an instance of that is added to the Qt Quick scene by the
    application.
*/

/*!
    \qmlproperty enumeration QtQuick3D::DebugSettings::materialOverride
    \since 6.5

    This property changes how all materials are rendered to only reflect a
    particular aspect of the overall rendering process. This can be used as
    a debugging tool to get a better understanding of why a material looks
    the way it does.

    The default value is \c DebugSettings.None

    \value DebugSettings.None
        Material overriding is bypassed, rendering occurs as normal.
    \value DebugSettings.BaseColor
        The BaseColor or Diffuse color of a material is passed through
        without any lighting.
    \value DebugSettings.Roughness
        The Roughness of a material is passed through as an unlit
        greyscale value.
    \value DebugSettings.Metalness
        The Metalness of a material is passed through as an unlit
        greyscale value.
    \value DebugSettings.Diffuse
        Only the diffuse contribution of the material after all lighting.
    \value DebugSettings.Specular
        Only the specular contribution of the material after all lighting.
    \value DebugSettings.ShadowOcclusion
        The Occlusion caused by shadows as a greyscale value.
    \value DebugSettings.Emission
        Only the emissive contribution of the material
    \value DebugSettings.AmbientOcclusion
        Only the Ambient Occlusion of the material
    \value DebugSettings.Normals
        The interpolated world space Normal value of the material mapped to an
        RGB color.
    \value DebugSettings.Tangents
        The interpolated world space Tangent value of the material mapped to an
        RGB color. This will only be visible if the Tangent value is used.
    \value DebugSettings.Binormals
        The interpolated world space Binormal value of the material mapped to an
        RGB color. This will only be visible if the Binormal value is used.
    \value DebugSettings.F0
        This represents the Fresnel Reflectance at 0 Degrees. This will only be
        visible for materials that calculate an F0 value.

    As an example, take the following scene with the
    \l{https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/Sponza}{Sponza}
    model. The scene uses image-based lighting via
    \l{SceneEnvironment::lightProbe} and also has a directional light.

    \image debugsettings_default.jpg
           {Scene with default debug settings}

    Setting \c{DebugSettings.BaseColor}:

    \image debugsettings_basecolor.jpg
           {Scene showing base colors without lighting}

    Setting \c{DebugSettings.Roughness}:

    \image debugsettings_roughness.jpg
           {Scene showing roughness values}

    Setting \c{DebugSettings.Metalness}:

    \image debugsettings_metalness.jpg
           {Scene highlighting metalness values}

    Setting \c{DebugSettings.Diffuse}:

    \image debugsettings_diffuse.jpg
           {Scene showing diffuse lighting only}

    Setting \c{DebugSettings.Specular}:

    \image debugsettings_specular.jpg
           {Scene showing specular lighting only}

    Setting \c{DebugSettings.Normals}:

    \image debugsettings_normals.jpg
           {Scene showing normal vectors as colors}
*/


QQuick3DDebugSettings::QQuick3DDebugSettings(QObject *parent)
    : QObject(parent)
{

}

QQuick3DDebugSettings::QQuick3DMaterialOverrides QQuick3DDebugSettings::materialOverride() const
{
    return m_materialOverride;
}

void QQuick3DDebugSettings::setMaterialOverride(QQuick3DMaterialOverrides newMaterialOverride)
{
    if (m_materialOverride == newMaterialOverride)
        return;
    m_materialOverride = newMaterialOverride;
    emit materialOverrideChanged();
    update();
}

void QQuick3DDebugSettings::update()
{
    emit changed();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::wireframeEnabled
    \since 6.5

    This property changes how all materials are rendered by changing the polygon
    fill mode to be lines instead of filled. This appears as a wireframe, but the
    shaded color will still reflect the respective materials of the meshes.

    The default value is \c false.

    \image debugsettings_wireframe.jpg
           {Scene showing wireframe rendering}
*/


bool QQuick3DDebugSettings::wireframeEnabled() const
{
    return m_wireframeEnabled;
}

void QQuick3DDebugSettings::setWireframeEnabled(bool newWireframeEnabled)
{
    if (m_wireframeEnabled == newWireframeEnabled)
        return;
    m_wireframeEnabled = newWireframeEnabled;
    emit wireframeEnabledChanged();
    update();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::drawDirectionalLightShadowBoxes
    \since 6.8

    When this property is enabled a bounding box is drawn for every directional light's shadowmap.

    The default value is \c false.
*/

bool QQuick3DDebugSettings::drawDirectionalLightShadowBoxes() const
{
    return m_drawDirectionalLightShadowBoxes;
}

void QQuick3DDebugSettings::setDrawDirectionalLightShadowBoxes(bool newDrawDirectionalLightShadowBoxes)
{
    if (m_drawDirectionalLightShadowBoxes == newDrawDirectionalLightShadowBoxes)
        return;
    m_drawDirectionalLightShadowBoxes = newDrawDirectionalLightShadowBoxes;
    emit drawDirectionalLightShadowBoxesChanged();
    update();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::drawShadowCastingBounds
    \since 6.8

    When this property is enabled a bounding box is drawn for the shadow casting objects.

    The default value is \c false.
*/

bool QQuick3DDebugSettings::drawShadowCastingBounds() const
{
    return m_drawShadowCastingBounds;
}

void QQuick3DDebugSettings::setDrawShadowCastingBounds(bool newDrawShadowCastingBounds)
{
    if (m_drawShadowCastingBounds == newDrawShadowCastingBounds)
        return;
    m_drawShadowCastingBounds = newDrawShadowCastingBounds;
    emit drawShadowCastingBoundsChanged();
    update();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::drawPointLightShadowBoxes
    \since 6.9

    When this property is enabled a bounding box is drawn for every point light's shadowmap.

    The default value is \c false.
*/

bool QQuick3DDebugSettings::drawPointLightShadowBoxes() const
{
    return m_drawPointLightShadowBoxes;
}

void QQuick3DDebugSettings::setDrawPointLightShadowBoxes(bool newDrawPointLightShadowBoxes)
{
    if (m_drawPointLightShadowBoxes == newDrawPointLightShadowBoxes)
        return;
    m_drawPointLightShadowBoxes = newDrawPointLightShadowBoxes;
    emit drawPointLightShadowBoxesChanged();
    update();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::drawShadowReceivingBounds
    \since 6.8

    When this property is enabled a bounding box is drawn for the shadow receiving objects.

    The default value is \c false.
*/

bool QQuick3DDebugSettings::drawShadowReceivingBounds() const
{
    return m_drawShadowReceivingBounds;
}

void QQuick3DDebugSettings::setDrawShadowReceivingBounds(bool newDrawShadowReceivingBounds)
{
    if (m_drawShadowReceivingBounds == newDrawShadowReceivingBounds)
        return;
    m_drawShadowReceivingBounds = newDrawShadowReceivingBounds;
    emit drawShadowReceivingBoundsChanged();
    update();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::drawCascades
    \since 6.8

    When this property is enabled a frustum is drawn with splits indicating where the
    shadowmap cascades begin and end.

    The default value is \c false.
*/

bool QQuick3DDebugSettings::drawCascades() const
{
    return m_drawCascades;
}

void QQuick3DDebugSettings::setDrawCascades(bool newDrawCascades)
{
    if (m_drawCascades == newDrawCascades)
        return;
    m_drawCascades = newDrawCascades;
    emit drawCascadesChanged();
    update();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::drawSceneCascadeIntersection
    \since 6.8

    When this property is enabled the intersection of the shadowmap cascades
    and the casting and receiving objects of the scene is drawn.

    The default value is \c false.
*/

bool QQuick3DDebugSettings::drawSceneCascadeIntersection() const
{
    return m_drawSceneCascadeIntersection;
}

void QQuick3DDebugSettings::setDrawSceneCascadeIntersection(bool newDrawSceneCascadeIntersection)
{
    if (m_drawSceneCascadeIntersection == newDrawSceneCascadeIntersection)
        return;
    m_drawSceneCascadeIntersection = newDrawSceneCascadeIntersection;
    emit drawSceneCascadeIntersectionChanged();
    update();
}

/*!
    \qmlproperty bool QtQuick3D::DebugSettings::disableShadowCameraUpdate
    \since 6.8

    When this property is enabled the camera update is disabled for the shadowmap.
    This means that the view frustum will be locked in space just for the shadowmap
    calculations. This is just a debug tool to be able to view the camera frustum
    and shadow map from different angles.

    The default value is \c false.
*/

bool QQuick3DDebugSettings::disableShadowCameraUpdate() const
{
    return m_disableShadowCameraUpdate;
}

void QQuick3DDebugSettings::setDisableShadowCameraUpdate(bool newDisableShadowCameraUpdate)
{
    if (m_disableShadowCameraUpdate == newDisableShadowCameraUpdate)
        return;
    m_disableShadowCameraUpdate = newDisableShadowCameraUpdate;
    emit disableShadowCameraUpdateChanged();
    update();
}
/*!
    \internal
    \qmlproperty bool QtQuick3D::DebugSettings::drawCulledObjects
    \since 6.11

    Draws a different-colored bounding box based on whether the model is culled.

    The default value is \c false.
*/
bool QQuick3DDebugSettings::drawCulledObjects() const
{
    return m_drawCulledObjects;
}

void QQuick3DDebugSettings::setDrawCulledObjects(bool newDrawCulledObjects)
{
    if (m_drawCulledObjects == newDrawCulledObjects)
        return;
    m_drawCulledObjects = newDrawCulledObjects;
    emit drawCulledObjectsChanged();
    update();
}

QT_END_NAMESPACE


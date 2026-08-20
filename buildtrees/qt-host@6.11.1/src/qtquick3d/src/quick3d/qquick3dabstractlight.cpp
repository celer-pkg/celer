// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dabstractlight_p.h"
#include "qquick3dobject_p.h"
#include "qquick3dnode_p_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Light
    \inherits Node
    \inqmlmodule QtQuick3D
    \brief An uncreatable abstract base type for all lights.

    Light itself is an uncreatable base for all of its subtypes. The subtypes provide multiple
    options to determine the style of the light.

    For usage examples, see \l{Qt Quick 3D - Lights Example}.

    \sa DirectionalLight, PointLight
*/

/*!
    \qmlproperty color Light::color
    This property defines the color applied to models illuminated by this light.
    The default value is white, rgb(255, 255, 255).
 */

/*!
    \qmlproperty color Light::ambientColor
    The property defines the ambient color applied to materials before being lit by this light.
    The default value is black, rgb(0, 0, 0).
 */

/*!
    \qmlproperty real Light::brightness
    This property defines an overall multiplier for this light’s effects.
    The default value is 1.
*/

/*!
    \qmlproperty Node Light::scope

    The property allows the selection of a Node in the scene. Only that node
    and its children are affected by this light. By default the value is null,
    which indicates no scope selected.

    \note Scoped lights cannot cast real-time shadows, meaning a Light with a
    scope set should not set \l castsShadow to true. They can however generate
    baked shadows when \l bakeMode is set to Light.BakeModeAll.
*/

/*!
    \qmlproperty bool Light::castsShadow

    When this property is enabled, the light will cast (real-time) shadows. The
    default value is false.

    \note When \l bakeMode is set to Light.BakeModeAll, this property has no
    effect. A fully baked light always has baked shadows, but it will never
    participate in real-time shadow mapping.
*/

/*!
    \qmlproperty real Light::shadowBias
    This property is used to tweak the shadowing effect when objects
    are casting shadows on themselves. The value tries to approximate the offset
    in world space so it needs to be tweaked depending on the size of your scene.

    The default value is \c{10}
*/

/*!
    \qmlproperty real Light::shadowFactor
    This property determines how dark the cast shadows should be. The value range is [0, 100], where
    0 means no shadows and 100 means the light is fully shadowed.

    The default value is \c{75}.
*/

/*!
    \qmlproperty enumeration Light::shadowMapQuality
    The property sets the quality of the shadow map created for shadow rendering. Lower quality uses
    less resources, but produces lower quality shadows while higher quality uses more resources, but
    produces better quality shadows.

    Supported quality values are:
    \value Light.ShadowMapQualityLow Render shadowmap using a 256x256 texture.
    \value Light.ShadowMapQualityMedium Render shadowmap using a 512x512 texture.
    \value Light.ShadowMapQualityHigh Render shadowmap using a 1024x1024 texture.
    \value Light.ShadowMapQualityVeryHigh Render shadowmap using a 2048x2048 texture.
    \value Light.ShadowMapQualityUltra Render shadowmap using a 4096x4096 texture.

    The default value is \c Light.ShadowMapQualityLow
*/

/*!
    \qmlproperty real Light::shadowMapFar
    The property determines the maximum distance for the shadow map. Smaller
    values improve the precision and effects of the map.
    The default value is 5000. Unit is points in local coordinate space.
*/

/*!
    \qmlproperty real Light::shadowFilter
    This property sets how much blur is applied to the shadows.

    The default value is 5.

    \deprecated [6.8] No longer used for anything, use \l{Light::}{pcfFactor} instead.

    \sa Light::softShadowQuality
*/

/*!
    \qmlproperty enumeration Light::bakeMode
    The property controls if the light is active in baked lighting, such as
    when generating lightmaps.

    \value Light.BakeModeDisabled The light is not used in baked lighting.

    \value Light.BakeModeIndirect Indirect lighting contribution (for global
    illumination) is baked for this light. Direct lighting (diffuse, specular,
    real-time shadow mapping) is calculated normally for the light at run time.
    At run time, when not in baking mode, the renderer will attempt to sample
    the lightmap to get the indirect lighting data and combine that with the
    results of the real-time calculations.

    \value Light.BakeModeAll Both direct (diffuse, shadow) and indirect
    lighting is baked for this light. The light will not have a specular
    contribution and will not generate realtime shadow maps, but it will always
    have baked shadows. At run time, when not in baking mode, the renderer will
    attempt to sample the lightmap in place of the standard, real-time
    calculations for diffuse lighting and shadow mapping.

    The default value is \c Light.BakeModeDisabled

    \note Just as with \l Model::usedInBakedLighting, designers and developers
    must always evaluate on a per-light basis if the light is suitable to take
    part in baked lighting.

    \warning Lights with dynamically changing properties, for example, animated
    position, rotation, or other properties, are not suitable for participating
    in baked lighting.

    This property is relevant both when baking and when using lightmaps. A
    consistent state between the baking run and the subsequent runs that use
    the generated data is essential. Changing to a different value will not
    change the previously generated and persistently stored data in the
    lightmaps, the engine's rendering behavior will however follow the
    property's current value.

    For more information on how to bake lightmaps, see the \l {Lightmaps and
    Global Illumination}.

    \sa Model::usedInBakedLighting, Model::bakedLightmap, Lightmapper, {Lightmaps and Global Illumination}
*/

/*!
    \qmlproperty enumeration Light::softShadowQuality
    \since 6.8

    The property suggests the soft shadow quality that should be used when
    shadows are enabled. The actual number of samples used when rendering shadows
    is determined by highest quality set by any light used by a material.

    \value Light.Hard No soft shadows.
    \value Light.PCF4 Percentage-closer filtering soft shadows with 4 samples.
    \value Light.PCF8 Percentage-closer filtering soft shadows with 8 samples.
    \value Light.PCF16 Percentage-closer filtering soft shadows with 16 samples.
    \value Light.PCF32 Percentage-closer filtering soft shadows with 32 samples.
    \value Light.PCF64 Percentage-closer filtering soft shadows with 64 samples.

    Default value: \c Light.PCF4

    \note Currently values larger than 16 samples will be treated as 16 samples.

    \sa Light::pcfFactor, Light::shadowFilter
*/

/*!
    \qmlproperty real Light::pcfFactor
    \since 6.8

    The property controls the PCF (percentage-closer filtering) factor. This
    value tries to approximate the radius of a PCF filtering in world space.

    \note PCF needs to be set in \l{Light::}{softShadowQuality} for this property
    to have an effect.

    Default value: \c{2.0}

    \sa Light::softShadowQuality
*/

/*!
    \qmlproperty bool Light::use32BitShadowmap
    \since 6.9

    The property controls if a 32-bit shadowmap depth buffer should be used for the light.

    Default value: \c{false}

    \sa Light::castsShadow
*/

QQuick3DAbstractLight::QQuick3DAbstractLight(QQuick3DNodePrivate &dd, QQuick3DNode *parent)
    : QQuick3DNode(dd, parent)
    , m_color(Qt::white)
    , m_ambientColor(Qt::black) {}

QQuick3DAbstractLight::~QQuick3DAbstractLight() {}

QColor QQuick3DAbstractLight::color() const
{
    return m_color;
}

QColor QQuick3DAbstractLight::ambientColor() const
{
    return m_ambientColor;
}

float QQuick3DAbstractLight::brightness() const
{
    return m_brightness;
}

QQuick3DNode *QQuick3DAbstractLight::scope() const
{
    return m_scope;
}

bool QQuick3DAbstractLight::castsShadow() const
{
    return m_castsShadow;
}

float QQuick3DAbstractLight::shadowBias() const
{
    return m_shadowBias;
}

float QQuick3DAbstractLight::shadowFactor() const
{
    return m_shadowFactor;
}

QQuick3DAbstractLight::QSSGShadowMapQuality QQuick3DAbstractLight::shadowMapQuality() const
{
    return m_shadowMapQuality;
}

QQuick3DAbstractLight::QSSGSoftShadowQuality QQuick3DAbstractLight::softShadowQuality() const
{
    return m_softShadowQuality;
}

float QQuick3DAbstractLight::shadowMapFar() const
{
    return m_shadowMapFar;
}

float QQuick3DAbstractLight::shadowFilter() const
{
    return m_shadowFilter;
}

QQuick3DAbstractLight::QSSGBakeMode QQuick3DAbstractLight::bakeMode() const
{
    return m_bakeMode;
}

float QQuick3DAbstractLight::pcfFactor() const
{
    return m_pcfFactor;
}

bool QQuick3DAbstractLight::use32BitShadowmap() const
{
    return m_use32BitShadowmap;
}

void QQuick3DAbstractLight::markAllDirty()
{
    m_dirtyFlags = DirtyFlags(DirtyFlag::ShadowDirty)
            | DirtyFlags(DirtyFlag::ColorDirty)
            | DirtyFlags(DirtyFlag::BrightnessDirty)
            | DirtyFlags(DirtyFlag::FadeDirty)
            | DirtyFlags(DirtyFlag::AreaDirty)
            | DirtyFlags(DirtyFlag::BakeModeDirty);
    QQuick3DNode::markAllDirty();
}

void QQuick3DAbstractLight::setColor(const QColor &color)
{
    if (m_color == color)
        return;

    m_color = color;
    m_dirtyFlags.setFlag(DirtyFlag::ColorDirty);
    emit colorChanged();
    update();
}

void QQuick3DAbstractLight::setAmbientColor(const QColor &ambientColor)
{
    if (m_ambientColor == ambientColor)
        return;

    m_ambientColor = ambientColor;
    m_dirtyFlags.setFlag(DirtyFlag::ColorDirty);
    emit ambientColorChanged();
    update();
}

void QQuick3DAbstractLight::setBrightness(float brightness)
{
    if (qFuzzyCompare(m_brightness, brightness))
        return;

    m_brightness = brightness;
    m_dirtyFlags.setFlag(DirtyFlag::BrightnessDirty);
    emit brightnessChanged();
    update();
}

void QQuick3DAbstractLight::setScope(QQuick3DNode *scope)
{
    if (m_scope == scope)
        return;

    m_scope = scope;
    emit scopeChanged();
    update();
}

void QQuick3DAbstractLight::setCastsShadow(bool castsShadow)
{
    if (m_castsShadow == castsShadow)
        return;

    m_castsShadow = castsShadow;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit castsShadowChanged();
    update();
}

void QQuick3DAbstractLight::setShadowBias(float shadowBias)
{
    if (qFuzzyCompare(m_shadowBias, shadowBias))
        return;

    m_shadowBias = shadowBias;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowBiasChanged();
    update();
}

void QQuick3DAbstractLight::setShadowFactor(float shadowFactor)
{
    shadowFactor = qBound(0.0f, shadowFactor, 100.0f);
    if (qFuzzyCompare(m_shadowFactor, shadowFactor))
        return;

    m_shadowFactor = shadowFactor;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowFactorChanged();
    update();
}

void QQuick3DAbstractLight::setShadowMapQuality(
        QQuick3DAbstractLight::QSSGShadowMapQuality shadowMapQuality)
{
    if (m_shadowMapQuality == shadowMapQuality)
        return;

    m_shadowMapQuality = shadowMapQuality;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowMapQualityChanged();
    update();
}

void QQuick3DAbstractLight::setSoftShadowQuality(QSSGSoftShadowQuality softShadowQuality)
{
    if (m_softShadowQuality == softShadowQuality)
        return;

    m_softShadowQuality = softShadowQuality;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit softShadowQualityChanged();
    update();
}

void QQuick3DAbstractLight::setBakeMode(QQuick3DAbstractLight::QSSGBakeMode bakeMode)
{
    if (m_bakeMode == bakeMode)
        return;

    m_bakeMode = bakeMode;
    m_dirtyFlags.setFlag(DirtyFlag::BakeModeDirty);
    emit bakeModeChanged();
    update();
}

void QQuick3DAbstractLight::setPcfFactor(float pcfFactor)
{
    if (m_pcfFactor == pcfFactor)
        return;

    m_pcfFactor = pcfFactor;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit pcfFactorChanged();
    update();
}

void QQuick3DAbstractLight::setUse32BitShadowmap(bool use32BitShadowmap)
{
    if (m_use32BitShadowmap == use32BitShadowmap)
        return;

    m_use32BitShadowmap = use32BitShadowmap;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit use32BitShadowmapChanged();
    update();
}

void QQuick3DAbstractLight::setShadowMapFar(float shadowMapFar)
{
    if (qFuzzyCompare(m_shadowMapFar, shadowMapFar))
        return;

    m_shadowMapFar = shadowMapFar;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowMapFarChanged();
    update();
}

void QQuick3DAbstractLight::setShadowFilter(float shadowFilter)
{
    if (qFuzzyCompare(m_shadowFilter, shadowFilter))
        return;

    m_shadowFilter = shadowFilter;
    m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty);
    emit shadowFilterChanged();
    update();
}

quint32 QQuick3DAbstractLight::mapToShadowResolution(QSSGShadowMapQuality quality)
{
    switch (quality) {
    case QSSGShadowMapQuality::ShadowMapQualityMedium:
        return 512;
    case QSSGShadowMapQuality::ShadowMapQualityHigh:
        return 1024;
    case QSSGShadowMapQuality::ShadowMapQualityVeryHigh:
        return 2048;
    case QSSGShadowMapQuality::ShadowMapQualityUltra:
        return 4096;
    default:
        break;
    }
    return 256;
}

QSSGRenderGraphObject *QQuick3DAbstractLight::updateSpatialNode(QSSGRenderGraphObject *node)
{
    Q_ASSERT_X(node, __FUNCTION__, "Node must have been created in parent class.");

    QQuick3DNode::updateSpatialNode(node);

    QSSGRenderLight *light = static_cast<QSSGRenderLight *>(node);

    if (m_dirtyFlags.toInt() != 0) // Some flag was set, so mark the light dirty!
        light->markDirty(QSSGRenderLight::DirtyFlag::LightDirty);

    if (m_dirtyFlags.testFlag(DirtyFlag::ColorDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::ColorDirty, false);
        light->m_diffuseColor = QSSGUtils::color::sRGBToLinear(m_color).toVector3D();
        light->m_specularColor = light->m_diffuseColor;
        light->m_ambientColor = QSSGUtils::color::sRGBToLinear(m_ambientColor).toVector3D();
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::BrightnessDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::BrightnessDirty, false);
        light->m_brightness = m_brightness;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::ShadowDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::ShadowDirty, false);
        light->m_castShadow = m_castsShadow;
        light->m_shadowBias = m_shadowBias;
        light->m_shadowFactor = m_shadowFactor;
        light->m_shadowMapRes = mapToShadowResolution(m_shadowMapQuality);
        light->m_softShadowQuality = static_cast<QSSGRenderLight::SoftShadowQuality>(m_softShadowQuality);
        light->m_shadowMapFar = m_shadowMapFar;
        light->m_shadowFilter = m_shadowFilter;
        light->m_pcfFactor = m_pcfFactor;
        light->m_use32BitShadowmap = m_use32BitShadowmap;
    }

    if (m_dirtyFlags.testFlag(DirtyFlag::BakeModeDirty)) {
        m_dirtyFlags.setFlag(DirtyFlag::BakeModeDirty, false);
        light->m_bakingEnabled = m_bakeMode != QSSGBakeMode::BakeModeDisabled;
        light->m_fullyBaked = m_bakeMode == QSSGBakeMode::BakeModeAll;
    }

    if (m_scope) {
        // Special case:
        // If the 'scope' is 'this' and this is the first call, then the spatial node is the one we just created.
        // This is not unlikely, as it can make sense to put all child nodes that should receive light under the light node...
        if (m_scope == this)
            light->m_scope = light;
        else
            light->m_scope = static_cast<QSSGRenderNode*>(QQuick3DObjectPrivate::get(m_scope)->spatialNode);
    } else {
        light->m_scope = nullptr;
    }

    return node;
}

QT_END_NAMESPACE

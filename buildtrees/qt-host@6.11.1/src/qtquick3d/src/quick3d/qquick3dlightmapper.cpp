// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dlightmapper_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Lightmapper
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Specifies lightmap baking settings for a scene.
    \since 6.4

    Used when baking direct and indirect lighting. Most of these settings are not
    relevant at other times, such as when using already generated lightmaps to
    render a scene. The exception is \l source, though this has a sensible default
    for development.

    On a successfull bake a single file will be generated at the value specified by
    \l source. This binary file contains the results of the bake, including the
    per-model lightmaps and the mesh files with lightmap-compatible UVs.
    The individual model data is accessed via \l BakedLightmap::key.

    The data contained in the resulting lightmap file is all tightly coupled
    to each other and to the current scene state. This means that any modifications
    to the original mesh files, Lightmapper settings or other scene changes will
    require a new bake to be executed to see the updated result.

    \note As of Qt 6.4, lightmap baking is in an early technology preview state.
    Changes to features, quality, and API are likely to happen in future releases.

    The Lightmapper object works in combination with:

    \list
    \li \l Model::bakedLightmap and the associated \l BakedLightmap,
    \li \l Model::usedInBakedLighting and \l Model::texelsPerUnit,
    \li \l Light::bakeMode,
    \li the engine's built-in lightmap baker.
    \endlist

    \sa {Lightmaps and Global Illumination},
        {Qt Quick 3D - Baked Lightmap Example},
        {Qt Quick 3D - SSGI Lightmap Example}
 */

/*!
    \qmlproperty real Lightmapper::opacityThreshold

    The opacity (alpha) threshold below which an object is ignored in ray -
    mesh intersections when calculating lighting via raytracing. When the
    opacity falls below the threshold, the model (submesh) will not occlude
    lights and thus will not generate shadows either.

    The default value is 0.5.

    \note The lightmapper takes the \l{PrincipledMaterial::opacity}{material's
    opacity} and the \l{PrincipledMaterial::baseColor}{baseColor alpha}
    combined with the \l{PrincipledMaterial::baseColorMap}{base color map's
    alpha} into account. Other sources of semi-transparency, such as the
    opacity map or alpha cut-off settings are ignored during the lightmap
    baking process.
 */

/*!
    \qmlproperty real Lightmapper::bias

    Raycasting bias used during baking. Adapt the value in case artifacts
    occur, for example in order to reduce undesired shadowing patterns. In many
    cases the default value is sufficient.

    The default value is 0.005.
 */

/*!
    \qmlproperty bool Lightmapper::adaptiveBiasEnabled

    Enables applying an additional, dynamic bias based on the surface normal.

    The default value is true.
 */

/*!
    \qmlproperty bool Lightmapper::indirectLightEnabled

    Normally there is no need to change this value. The default value is true.
    Setting this property to false disables indirect light computation during
    lightmap baking. Thus the resulting texture maps will only contain direct
    light information. At run time, the engine will continue to use the maps
    normally, assuming they contain both direct and indirect lighting.
 */

/*!
    \qmlproperty int Lightmapper::samples

    The number of samples per lightmap texel.

    The default value is 256.

    The value heavily affects both the performance and quality of the resulting
    lightmaps during lightmap baking.
 */

/*!
    \qmlproperty int Lightmapper::indirectLightWorkgroupSize

    The size of the sample workgroups. These workgroups are attempted to be
    executed in parallel. (the exact behavior depends on the number of CPU
    cores and the QThreadPool configuration)

    The default value is 32. With the default sample count of 256 this means
    attempting to run 8 groups in parallel per model.
 */

/*!
    \qmlproperty int Lightmapper::bounces

    The maximum number of indirect light bounces per sample. The value should
    at least be 1, no point in indirect light calculation otherwise.

    The default value is 3.

    The value heavily affects both the performance and quality of the resulting
    lightmaps during lightmap baking.
*/

/*!
    \qmlproperty real Lightmapper::indirectLightFactor

    Multiplier for the indirect light amount. While it is the value of 1 (i.e.,
    not affecting the indirect light amount calculation) that provides the
    strictly correct rendering results, a slightly higher value can often give
    better looking results when using the lightmap, even with a lower number of
    bounces.

    The default value is 1.
 */

/*!
    \qmlproperty url Lightmapper::source
    \since 6.10
    \default file:lightmaps.bin

    The url for loading the lightmap file at runtime and the output file path
    on a successful bake.

    When baking, the output path will be deduced from the url and it needs to
    resolve to a regular file location that is writable. By default the value
    is \c{file:lightmaps.bin}, which means the file will be put in the current
    working directory and the final result will instantly appear on a successful
    bake.

    In the same directory as the output file, a \c{.raw} file is created that
    contains extra lightmap data used before creating the final lightmap file.
    This makes it possible to do just denoising without having to bake the whole
    scene between runs, assuming the \c{.raw} file is present.

    If you want to read the lightmap as a QRC resource you need to embed
    it in the usual way and add a \c{:/} or \c{qrc:/} prefix to the url.
    The following example always tries to load the lightmap file embedded via
    resources. First set the value to a writable location and bake. Then copy the
    generated file into the source directoy. Then by listing the file in the
    application's CMake project as a resource under the \c{/lightmaps} PREFIX,
    lets the build process pick up the file and include it in the executable.

    \qml
    Lightmapper {
        source: "qrc:/lightmaps/lightmaps.bin"
        // will attempt to load from :/lightmaps/lightmaps.bin at runtime
        // and write a file to lightmaps/lightmaps.bin when baking.
    }
    \endqml
 */

/*!
    \qmlproperty real Lightmapper::denoiseSigma
    \since 6.10
    \default 8

    This property defines the sigma value of the Non-local means based denoiser.
    This means that the higher this value is the stronger the blurring will be.
    Try to keep this value as low as possible to avoid losing visual features
    while still removing the noise.
*/

/*!
    \qmlproperty real Lightmapper::texelsPerUnit
    \since 6.10
    \default 1

    This property defines the unit to texel scale, meaning a \c{1x1} quad with
    texelsPerUnit of \c{32} will take up approximately \c{32x32} texels in the
    lightmap.

    \sa Model::texelsPerUnit
*/

float QQuick3DLightmapper::opacityThreshold() const
{
    return m_opacityThreshold;
}

float QQuick3DLightmapper::bias() const
{
    return m_bias;
}

bool QQuick3DLightmapper::isAdaptiveBiasEnabled() const
{
    return m_adaptiveBias;
}

bool QQuick3DLightmapper::isIndirectLightEnabled() const
{
    return m_indirectLight;
}

int QQuick3DLightmapper::samples() const
{
    return m_samples;
}

int QQuick3DLightmapper::indirectLightWorkgroupSize() const
{
    return m_workgroupSize;
}

int QQuick3DLightmapper::bounces() const
{
    return m_bounces;
}

float QQuick3DLightmapper::indirectLightFactor() const
{
    return m_indirectFactor;
}

QUrl QQuick3DLightmapper::source() const
{
    return m_source;
}

void QQuick3DLightmapper::setOpacityThreshold(float opacity)
{
    if (m_opacityThreshold == opacity)
        return;

    m_opacityThreshold = opacity;
    emit opacityThresholdChanged();
    emit changed();
}

void QQuick3DLightmapper::setBias(float bias)
{
    if (m_bias == bias)
        return;

    m_bias = bias;
    emit biasChanged();
    emit changed();
}

void QQuick3DLightmapper::setAdaptiveBiasEnabled(bool enabled)
{
    if (m_adaptiveBias == enabled)
        return;

    m_adaptiveBias = enabled;
    emit adaptiveBiasEnabledChanged();
    emit changed();
}

void QQuick3DLightmapper::setIndirectLightEnabled(bool enabled)
{
    if (m_indirectLight == enabled)
        return;

    m_indirectLight = enabled;
    emit indirectLightEnabledChanged();
    emit changed();
}

void QQuick3DLightmapper::setSamples(int count)
{
    if (m_samples == count)
        return;

    m_samples = count;
    emit samplesChanged();
    emit changed();
}

void QQuick3DLightmapper::setIndirectLightWorkgroupSize(int size)
{
    if (m_workgroupSize == size)
        return;

    m_workgroupSize = size;
    emit indirectLightWorkgroupSizeChanged();
    emit changed();
}

void QQuick3DLightmapper::setBounces(int count)
{
    if (m_bounces == count)
        return;

    m_bounces = count;
    emit bouncesChanged();
    emit changed();
}

void QQuick3DLightmapper::setIndirectLightFactor(float factor)
{
    if (m_indirectFactor == factor)
        return;

    m_indirectFactor = factor;
    emit indirectLightFactorChanged();
    emit changed();
}

void QQuick3DLightmapper::setSource(const QUrl &source)
{
    if (m_source == source)
        return;

    m_source = source;
    emit sourceChanged();
    emit changed();
}

float QQuick3DLightmapper::denoiseSigma() const
{
    return m_denoiseSigma;
}

void QQuick3DLightmapper::setDenoiseSigma(float newDenoiseSigma)
{
    if (qFuzzyCompare(m_denoiseSigma, newDenoiseSigma))
        return;
    m_denoiseSigma = newDenoiseSigma;
    emit denoiseSigmaChanged();
    emit changed();
}

float QQuick3DLightmapper::texelsPerUnit() const
{
    return m_texelsPerUnit;
}

void QQuick3DLightmapper::setTexelsPerUnit(float newTexelsPerUnit)
{
    if (qFuzzyCompare(m_texelsPerUnit, newTexelsPerUnit))
        return;
    m_texelsPerUnit = newTexelsPerUnit;
    emit texelsPerUnitChanged();
    emit changed();
}

QT_END_NAMESPACE

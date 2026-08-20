// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dspecularglossymaterial_p.h"
#include "qquick3dobject_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype SpecularGlossyMaterial
    \inherits Material
    \inqmlmodule QtQuick3D
    \brief Lets you define a material for 3D items using the specular/glossiness workflow.

    Before a Model can be rendered in a scene, it must have at least one material attached
    to it that describes how the mesh should be shaded. The SpecularGlossyMaterial is a PBR
    specular/glossiness material that aims at being an easy to use material with a minimal
    set of parameters.
    In addition to having few parameters, all input values are strictly normalized
    between 0 and 1 and have sensible defaults, meaning even without changing any values,
    the material can be used to shader a model. For an introduction on how the
    different properties of the SpecularGlossyMaterial affects how a model is shaded,
    see the \l{Qt Quick 3D - Principled Material Example}{Principled Material example} which
    provides a mode for using the Specular/Glossy workflow.

    Alternatively to use the metallic/roughness workflow instead use \l {PrincipledMaterial}.

    \section1 Specular/Glossiness workflow

    The SpecularGlossyMaterial provides a way to create materials using a specular/glossiness
    type workflow. The main properties of the material is controlled through
    the \l {SpecularGlossyMaterial::specularMap} {specular},
    \l {SpecularGlossyMaterial::glossinessMap} {glossiness},
    and \l {SpecularGlossyMaterial::albedoMap} {albedo} properties.

    \section2 Specular

    The \l {SpecularGlossyMaterial::specularMap} {specularColor} property describes the specular
    amount and color of an object's surface. For reflective materials the main color
    contribution, comes from this property.

    \section2 Glossiness

    The \l {SpecularGlossyMaterial::glossinessMap} {glossiness} of a material describes the
    condition of an object's surface. A high \c glossiness value means the object has a smooth
    surface and therefore be more reflective then a material with a lower \c glossiness value.

    \section2 Albedo

    The \l {SpecularGlossyMaterial::albedoMap} {albedoColor} property describes the diffuse color
    of the material, and unlike the \l {PrincipledMaterial}'s base color, the albedo
    does not contain any information about the material reflectance. That means that for
    reflective surfaces the albedo's color value should be set to a black tint, as that
    will allow for the specular color to contribute.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::lighting

    This property defines which lighting method is used when generating this
    material.

    The default value is \c SpecularGlossyMaterial.FragmentLighting

    \value SpecularGlossyMaterial.FragmentLighting Diffuse and specular lighting is
    calculated for each rendered pixel. Certain effects (such as a Fresnel or normal map) require
    \c SpecularGlossyMaterial.FragmentLighting to work.

    \value SpecularGlossyMaterial.NoLighting No lighting is calculated. This
    mode is (predictably) very fast, and is quite effective when image maps are
    used that you do not need to be shaded by lighting. All other shading
    properties except albedo values, alpha values, and vertex colors will be
    ignored.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::blendMode

    This property determines how the colors of the model rendered blends with
    those behind it.

    \value SpecularGlossyMaterial.SourceOver Default blend mode. Opaque objects
    occlude objects behind them. This default mode does not guarantee alpha
    blending in the rendering pipeline on its own for models that use this
    material, but rather makes the decision dependent on a number of factors:
    if the object's and material's total opacity is \c{1.0}, there is no
    opacity map in the material, and \l alphaMode is not set to a value that
    enforces alpha blending, then the model is treated as opaque, meaning it is
    rendered with depth testing and depth write enabled, together with other
    opaque objects, with blending disabled. Otherwise the model is treated as
    semi-transparent, and is rendered after the opaque objects, together with
    other semi-transparent objects in a back-to-front order based on their
    center's distance from the camera, with alpha blending enabled.

    \value SpecularGlossyMaterial.Screen Colors are blended using an inverted
    multiply, producing a lighter result. This blend mode is order-independent;
    if you are using semi-opaque objects and experiencing 'popping' as faces or
    models sort differently, using Screen blending is one way to produce
    results without popping.

    \value SpecularGlossyMaterial.Multiply Colors are blended using a multiply,
    producing a darker result. This blend mode is also order-independent.

    \sa alphaMode, {Qt Quick 3D Architecture}
*/

/*!
    \qmlproperty color SpecularGlossyMaterial::albedoColor

    This property sets the albedo color for the material. Setting the
    diffuse color to a black tint will create a purely-specular material
    (e.g. metals or mirrors).

    \sa albedoMap, alphaMode
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::albedoMap

    This property defines the texture used to set the albedo color of the material.

    \sa albedo, alphaMode
*/

/*!
    \qmlproperty bool SpecularGlossyMaterial::albedoSingleChannelEnabled
    \since 6.8

    When this property is enabled, the material will use the single value of the albedoChannel from
    the albedoMap as RGB value and use 1.0 as alpha value.
    The default value is false.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::albedoChannel
    \since 6.8

    This property defines the texture channel used to read the albedo color value from albedoMap.
    In order to use a single texture channel as color you have to enable the albedoSingleChannelEnabled
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty color SpecularGlossyMaterial::specularColor

    This property defines the specular RGB color. If an alpha value is provided it
    will be ignored.

    The default value is \c Qt.White
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::specularMap

    This property sets a Texture to be used to set the specular color for the
    different parts of the material. Only the RGB channels are used.
*/

/*!
    \qmlproperty bool SpecularGlossyMaterial::specularSingleChannelEnabled
    \since 6.8

    When this property is enabled, the material will use the single value of the specularChannel from
    the specularMap as RGB value.
    The default value is false.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::specularChannel
    \since 6.8

    This property defines the texture channel used to read the specular color value from specularMap.
    In order to use a single texture channel as color you have to enable the specularSingleChannelEnabled
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::glossiness

    This property controls the size of the specular highlight generated from
    lights, and the clarity of reflections in general. Smaller values increase
    the roughness, softening specular highlights and blurring reflections.
    The range is [0.0, 1.0]. The default value is \c 1.0.
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::glossinessMap

    This property defines a Texture to control the glossiness of the
    material.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::glossinessChannel

    This property defines the texture channel used to read the glossiness value
    from glossinessMap.
    The default value is \c Material.A.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty bool SpecularGlossyMaterial::emissiveSingleChannelEnabled
    \since 6.8

    When this property is enabled, the material will use the single value of the emissiveChannel from
    the emissiveMap as RGB value.
    The default value is false.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::emissiveChannel
    \since 6.8

    This property defines the texture channel used to read the emissive color value from emissiveMap.
    In order to use a single texture channel as color you have to enable the emissiveSingleChannelEnabled
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::emissiveMap

    This property sets a RGB Texture to be used to specify the intensity of the
    emissive color.
*/

/*!
    \qmlproperty vector3d SpecularGlossyMaterial::emissiveFactor

    This property determines the color of self-illumination for this material.
    If an emissive map is set, the x, y, and z components are used as factors
    (multipliers) for the R, G and B channels of the texture, respectively.
    The default value is (0, 0, 0) and it means no emissive contribution at all.

    \note Setting the lightingMode to DefaultMaterial.NoLighting means emissive
    Factor does not have an effect on the scene.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::opacity

    This property drops the opacity of just this material, separate from the
    model.
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::opacityMap

    This property defines a Texture used to control the opacity differently for
    different parts of the material.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::invertOpacityMapValue
    \since 6.8

    This property inverts the opacity value of the opacityMap.
    The default value is \c false.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::opacityChannel

    This property defines the texture channel used to read the opacity value from opacityMap.
    The default value is \c Material.A.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::normalMap

    This property defines an RGB image used to simulate fine geometry
    displacement across the surface of the material. The RGB channels indicate
    XYZ normal deviations.

    \note Normal maps will not affect the silhouette of a model.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::normalStrength

    This property controls the amount of simulated displacement for the normalMap.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::occlusionAmount

    This property contains the factor used to modify the values from the \l occlusionMap texture.
    The value should be between 0.0 to 1.0. The default is 1.0
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::occlusionMap

    This property defines a texture used to determine how much indirect light the different areas of the
    material should receive. Values are expected to be linear from 0.0 to 1.0, where 0.0 means no indirect lighting
    and 1.0 means the effect of the indirect lighting is left unchanged.

    \sa occlusionAmount
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::occlusionChannel

    This property defines the texture channel used to read the occlusion value from occlusionMap.
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::alphaMode

    This property specifies how the alpha color value from \l albedoColor and the
    alpha channel of a \l{albedoMap}{albedo map} are used.

    \note The alpha cutoff test only considers the albedo color alpha. \l opacity
    and \l [QtQuick3D] {Node::opacity} are not taken into account there.

    \note When sampling an albedo color map, the effective alpha value is the
    sampled alpha multiplied by the \l albedoColor alpha.

    \value SpecularGlossyMaterial.Default No test is applied, the effective alpha
    value is passed on as-is. Note that a \l albedoColor or \l albedoMap alpha
    less than \c 1.0 does not automatically imply alpha blending, the object
    with the material may still be treated as opaque, if no other relevant
    properties (such as, an opacity less than 1, the presence of an opacity
    map, or a non-default \l blendMode value) trigger treating the object as
    semi-transparent. To ensure alpha blending happens regardless of any other
    object or material property, set \c Blend instead.

    \value SpecularGlossyMaterial.Blend No cutoff test is applied, but guarantees
    that alpha blending happens. The object with this material will therefore
    never be treated as opaque by the renderer.

    \value SpecularGlossyMaterial.Opaque No cutoff test is applied and the rendered
    object is assumed to be fully opaque, meaning the alpha values in the
    vertex color, albedo color, and albedo color map are ignored and a value of 1.0
    is substituted instead. This mode does not guarantee alpha blending does
    not happen. If relevant properties (such as, an opacity less than 1, an
    opacity map, or a non-default \l blendMode) say so, then the object will
    still be treated as semi-transparent by the rendering pipeline, just like
    with the \c Default alphaMode.

    \value SpecularGlossyMaterial.Mask A test based on \l alphaCutoff is applied.
    If the effective alpha value falls below \l alphaCutoff, the fragment is
    changed to fully transparent and is discarded (with all implications of
    discarding: the depth buffer is not written for that fragment). Otherwise
    the alpha is changed to 1.0, so that the fragment will become fully opaque.
    When it comes to alpha blending, the behavior of this mode is identical to
    \c Opaque, regardless of the cutoff test's result. This means that the
    \l{https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#alpha-coverage}{glTF
    2 spec's alpha coverage} Implementation Notes are fulfilled. Objects with
    alpha cutoff tests can also cast shadows since they behave like opaque
    objects by default, unless the relevant properties (such as, an opacity
    less than 1, an opacity map, or a non-default \l blendMode) imply otherwise
    (in which case casting shadows will not be possible).

    \sa alphaCutoff, blendMode
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::alphaCutoff

    The alphaCutoff property can be used to specify the cutoff value when using
    the \l{alphaMode}{Mask alphaMode}. Fragments where the alpha value falls
    below the threshold will be rendered fully transparent (\c{0.0} for all
    color channels). When the alpha value is equal or greater than the cutoff
    value, the color will not be affected in any way.

    The default value is 0.5.

    \sa alphaMode
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::pointSize

    This property determines the size of the points rendered, when the geometry
    is using a primitive type of points. The default value is 1.0. This
    property is not relevant when rendering other types of geometry, such as,
    triangle meshes.

    \warning Point sizes other than 1 may not be supported at run time,
    depending on the underyling graphics API. For example, setting a size other
    than 1 has no effect with Direct 3D.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::lineWidth

    This property determines the width of the lines rendered, when the geometry
    is using a primitive type of lines or line strips. The default value is
    1.0. This property is not relevant when rendering other types of geometry,
    such as, triangle meshes.

    \warning Line widths other than 1 may not be suported at run time,
    depending on the underlying graphics API. When that is the case, the
    request to change the width is ignored. For example, none of the following
    can be expected to support wide lines: Direct3D, Metal, OpenGL with core
    profile contexts.
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::heightMap

    This property defines a texture used to determine the height the texture
    will be displaced when rendered through the use of Parallax Mapping. Values
    are expected to be linear from 0.0 to 1.0, where 0.0 means no displacement
    and 1.0 means means maximum displacement.

*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::heightChannel

    This property defines the texture channel used to read the height value
    from heightMap. The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real SpecularGlossyMaterial::heightAmount

    This property contains the factor used to modify the values from the
    \l heightMap texture. The value should be between 0.0 to 1.0. The default
    value is 0.0 which means that height displacement will be disabled, even
    if a height map set.
*/

/*!
    \qmlproperty int SpecularGlossyMaterial::minHeightMapSamples

    This property defines the minimum number of samples used for performing
    Parallex Occlusion Mapping using the \l heightMap. The minHeightMapSamples
    value is the number of samples of the heightMap are used when looking directly
    at a surface (when the camera view is perpendicular to the fragment).
    The default value is 8.

    The actual number of samples used for each fragment will be between
    \l minHeightMapSamples and \l maxHeightMapSamples depending on the angle of
    the camera relative to the surface being rendered.

    \note This value should only be adjusted to fine tune materials using a
    \l heightMap in the case undesired artifacts are present.
*/

/*!
    \qmlproperty int SpecularGlossyMaterial::maxHeightMapSamples

    This property defines the maximum number of samples used for performing
    Parallex Occlusion Mapping using the \l heightMap. The maxHeightMapSamples
    value is the number of samples of the heightMap are used when looking
    parallel to a surface.
    The default value is 32.

    The actual number of samples used for each fragment will be between
    \l minHeightMapSamples and \l maxHeightMapSamples depending on the angle of
    the camera relative to the surface being rendered.

    \note This value should only be adjusted to fine tune materials using a
    \l heightMap in the case undesired artifacts are present.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::clearcoatAmount

    This property defines the intensity of the clearcoat layer.

    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::clearcoatMap

    This property defines a texture used to determine the intensity of the
    clearcoat layer.  The value of\l clearcoatAmount will be multiplied by
    the value read from this texture.

*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::clearcoatChannel

    This property defines the texture channel used to read the clearcoat amount
    value from \l clearcoatMap. The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real SpecularGlossyMaterial::clearcoatRoughnessAmount

    This property defines the roughness of the clearcoat layer.
    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::clearcoatRoughnessMap

    This property defines a texture used to determine the roughness of the
    clearcoat layer.  The value of\l clearcoatRoughnessAmount will be
    multiplied by the value read from this texture.

*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::clearcoatRoughnessChannel

    This property defines the texture channel used to read the clearcoat
    roughness amount from \l clearcoatRoughnessMap.
    The default value is \c Material.G.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::clearcoatNormalMap

    This property defines a texture used to determine the normal mapping
    applied to the clearcoat layer.

*/

/*!
    \qmlproperty real SpecularGlossyMaterial::transmissionFactor

    This property defines the percentage of light that is transmitted through
    the material's surface.
    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::transmissionMap

    This property defines a texture used to determine percentage of light that
    is transmitted through the surface..  The value of
    \l transmissionFactor will be multiplied by the value read from this
    texture.

*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::transmissionChannel

    This property defines the texture channel used to read the transmission
    percentage from \l transmissionMap.
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real SpecularGlossyMaterial::thicknessFactor

    This property defines the thickness of the volume beneath the surface.
    Unlike many other properties of SpecularGlossyMaterial, the value in defined
    in thicknessFactor is a value from 0.0 to +infinity for thickness in the
    models coordinate space.  A value of 0.0 means that the material is
    thin-walled.
    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture SpecularGlossyMaterial::thicknessMap

    This property defines a texture used to define the thickness of a
    material volume.  The value of \l thicknessFactor will be multiplied by the
    value read from this texture.

*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::thicknessChannel

    This property defines the texture channel used to read the thickness
    amount from \l transmissionMap.
    The default value is \c Material.G.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real SpecularGlossyMaterial::attenuationDistance

    This property defines the Density of the medium given as the average
    distance that light travels in the medium before interacting with a
    particle. The value is given in world space.
    The default value is \c +infinity.
*/

/*!
    \qmlproperty color SpecularGlossyMaterial::attenuationColor

    This property defines the color that white lights turns into due to
    absorption when reaching the attenuation distance.
    The default value is \c Qt.White

*/

/*!
    \qmlproperty real SpecularGlossyMaterial::fresnelScaleBiasEnabled

     By Setting the value to true the material will take Fresnel Scale and Fresnel Bias into account.
     The default value is \c false.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::fresnelScale

     This property scale head-on reflections (looking directly at the
     surface) while maintaining reflections seen at grazing angles.
     In order to affect changes to the material you have to enable fresnelScaleBiasEnabled.
     The default value is \c 1.0.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::fresnelBias

     This property push forward head-on reflections (looking directly at the
     surface) while maintaining reflections seen at grazing angles.
     In order to affect changes to the material you have to enable fresnelScaleBiasEnabled.
     The default value is \c 0.0.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::fresnelPower

     This property decreases head-on reflections (looking directly at the
     surface) while maintaining reflections seen at grazing angles.
     The default value is \c 5.0.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::clearcoatFresnelScaleBiasEnabled

    By Setting the value to true the material will take Clearcoat Fresnel Scale and Clearcoat Fresnel Bias into account.
    The default value is \c false.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::clearcoatFresnelScale

    This property scale head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    In order to affect changes to the material you have to enable clearcoatFresnelScaleBiasEnabled.
    The default value is \c 1.0.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::clearcoatFresnelBias

    This property push forward head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    In order to affect changes to the material you have to enable clearcoatFresnelScaleBiasEnabled.
    The default value is \c 0.0.
*/

/*!
    \qmlproperty real SpecularGlossyMaterial::clearcoatFresnelPower

    This property decreases head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    The default value is \c 5.0.
*/

/*!
    \qmlproperty bool SpecularGlossyMaterial::vertexColorsEnabled
    \since 6.5

    When this property is enabled, the material will use vertex colors from the
    mesh. These will be multiplied by any other colors specified for the
    material. The default value is true.
*/

/*!
    \qmlproperty bool SpecularGlossyMaterial::vertexColorsMaskEnabled
    \since 6.8

    When this property is enabled, the material will use vertex colors from the
    mesh as mask of various properties e.g Glossiness, OcclusionAmount, ... .
    The default value is false.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::vertexColorRedMask
    \since 6.8

    This property defines the vertex color red channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c SpecularGlossyMaterial.NoMask.

    \value SpecularGlossyMaterial.NoMask.
    \value SpecularGlossyMaterial.GlossinessMask.
    \value SpecularGlossyMaterial.NormalStrengthMask.
    \value SpecularGlossyMaterial.ClearcoatAmountMask.
    \value SpecularGlossyMaterial.ClearcoatRoughnessAmountMask.
    \value SpecularGlossyMaterial.ClearcoatNormalStrengthMask.
    \value SpecularGlossyMaterial.HeightAmountMask.
    \value SpecularGlossyMaterial.OcclusionAmountMask.
    \value SpecularGlossyMaterial.ThicknessFactorMask.
    \value SpecularGlossyMaterial.TransmissionFactorMask.

*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::vertexColorGreenMask
    \since 6.8

    This property defines the vertex color green channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c SpecularGlossyMaterial.NoMask.

    \value SpecularGlossyMaterial.NoMask.
    \value SpecularGlossyMaterial.GlossinessMask.
    \value SpecularGlossyMaterial.NormalStrengthMask.
    \value SpecularGlossyMaterial.ClearcoatAmountMask.
    \value SpecularGlossyMaterial.ClearcoatRoughnessAmountMask.
    \value SpecularGlossyMaterial.ClearcoatNormalStrengthMask.
    \value SpecularGlossyMaterial.HeightAmountMask.
    \value SpecularGlossyMaterial.OcclusionAmountMask.
    \value SpecularGlossyMaterial.ThicknessFactorMask.
    \value SpecularGlossyMaterial.TransmissionFactorMask.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::vertexColorBlueMask
    \since 6.8

    This property defines the vertex color blue channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c SpecularGlossyMaterial.NoMask.

    \value SpecularGlossyMaterial.NoMask.
    \value SpecularGlossyMaterial.GlossinessMask.
    \value SpecularGlossyMaterial.NormalStrengthMask.
    \value SpecularGlossyMaterial.ClearcoatAmountMask.
    \value SpecularGlossyMaterial.ClearcoatRoughnessAmountMask.
    \value SpecularGlossyMaterial.ClearcoatNormalStrengthMask.
    \value SpecularGlossyMaterial.HeightAmountMask.
    \value SpecularGlossyMaterial.OcclusionAmountMask.
    \value SpecularGlossyMaterial.ThicknessFactorMask.
    \value SpecularGlossyMaterial.TransmissionFactorMask.
*/

/*!
    \qmlproperty enumeration SpecularGlossyMaterial::vertexColorAlphaMask
    \since 6.8

    This property defines the vertex color alpha channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c SpecularGlossyMaterial.NoMask.

    \value SpecularGlossyMaterial.NoMask.
    \value SpecularGlossyMaterial.GlossinessMask.
    \value SpecularGlossyMaterial.NormalStrengthMask.
    \value SpecularGlossyMaterial.ClearcoatAmountMask.
    \value SpecularGlossyMaterial.ClearcoatRoughnessAmountMask.
    \value SpecularGlossyMaterial.ClearcoatNormalStrengthMask.
    \value SpecularGlossyMaterial.HeightAmountMask.
    \value SpecularGlossyMaterial.OcclusionAmountMask.
    \value SpecularGlossyMaterial.ThicknessFactorMask.
    \value SpecularGlossyMaterial.TransmissionFactorMask.
*/

QQuick3DSpecularGlossyMaterial::QQuick3DSpecularGlossyMaterial(QQuick3DObject *parent)
    : QQuick3DMaterial(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::SpecularGlossyMaterial)), parent)
{}

QQuick3DSpecularGlossyMaterial::~QQuick3DSpecularGlossyMaterial()
{
}

QQuick3DSpecularGlossyMaterial::Lighting QQuick3DSpecularGlossyMaterial::lighting() const
{
    return m_lighting;
}

QQuick3DSpecularGlossyMaterial::BlendMode QQuick3DSpecularGlossyMaterial::blendMode() const
{
    return m_blendMode;
}

QColor QQuick3DSpecularGlossyMaterial::albedoColor() const
{
    return m_albedo;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::albedoMap() const
{
    return m_albedoMap;
}

bool QQuick3DSpecularGlossyMaterial::albedoSingleChannelEnabled() const
{
    return m_albedoSingleChannelEnabled;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::albedoChannel() const
{
    return m_albedoChannel;
}

bool QQuick3DSpecularGlossyMaterial::specularSingleChannelEnabled() const
{
    return m_specularSingleChannelEnabled;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::specularChannel() const
{
    return m_specularChannel;
}

bool QQuick3DSpecularGlossyMaterial::emissiveSingleChannelEnabled() const
{
    return m_emissiveSingleChannelEnabled;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::emissiveChannel() const
{
    return m_emissiveChannel;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::emissiveMap() const
{
    return m_emissiveMap;
}

QVector3D QQuick3DSpecularGlossyMaterial::emissiveFactor() const
{
    return m_emissiveFactor;
}

float QQuick3DSpecularGlossyMaterial::glossiness() const
{
    return m_glossiness;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::glossinessMap() const
{
    return m_glossinessMap;
}

bool QQuick3DSpecularGlossyMaterial::invertOpacityMapValue() const
{
    return m_invertOpacityMapValue;
}

float QQuick3DSpecularGlossyMaterial::opacity() const
{
    return m_opacity;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::opacityMap() const
{
    return m_opacityMap;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::normalMap() const
{
    return m_normalMap;
}

QColor QQuick3DSpecularGlossyMaterial::specularColor() const
{
    return m_specular;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::specularMap() const
{
    return m_specularMap;
}

float QQuick3DSpecularGlossyMaterial::normalStrength() const
{
    return m_normalStrength;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::occlusionMap() const
{
    return m_occlusionMap;
}

float QQuick3DSpecularGlossyMaterial::occlusionAmount() const
{
    return m_occlusionAmount;
}

QQuick3DSpecularGlossyMaterial::AlphaMode QQuick3DSpecularGlossyMaterial::alphaMode() const
{
    return m_alphaMode;
}

float QQuick3DSpecularGlossyMaterial::alphaCutoff() const
{
    return m_alphaCutoff;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::glossinessChannel() const
{
    return m_glossinessChannel;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::opacityChannel() const
{
    return m_opacityChannel;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::occlusionChannel() const
{
    return m_occlusionChannel;
}

float QQuick3DSpecularGlossyMaterial::pointSize() const
{
    return m_pointSize;
}

float QQuick3DSpecularGlossyMaterial::lineWidth() const
{
    return m_lineWidth;
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::heightMap() const
{
    return m_heightMap;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::heightChannel() const
{
    return m_heightChannel;
}

float QQuick3DSpecularGlossyMaterial::heightAmount() const
{
    return m_heightAmount;
}

int QQuick3DSpecularGlossyMaterial::minHeightMapSamples() const
{
    return m_minHeightMapSamples;
}

int QQuick3DSpecularGlossyMaterial::maxHeightMapSamples() const
{
    return m_maxHeightMapSamples;
}

void QQuick3DSpecularGlossyMaterial::markAllDirty()
{
    m_dirtyAttributes = 0xffffffff;
    QQuick3DMaterial::markAllDirty();
}

void QQuick3DSpecularGlossyMaterial::setLighting(QQuick3DSpecularGlossyMaterial::Lighting lighting)
{
    if (m_lighting == lighting)
        return;

    m_lighting = lighting;
    emit lightingChanged();
    markDirty(LightingModeDirty);
}

void QQuick3DSpecularGlossyMaterial::setBlendMode(QQuick3DSpecularGlossyMaterial::BlendMode blendMode)
{
    if (m_blendMode == blendMode)
        return;

    m_blendMode = blendMode;
    emit blendModeChanged();
    markDirty(BlendModeDirty);
}

void QQuick3DSpecularGlossyMaterial::setAlbedoColor(const QColor &diffuseColor)
{
    if (m_albedo == diffuseColor)
        return;

    m_albedo = diffuseColor;
    emit albedoColorChanged();
    markDirty(AlbedoDirty);
}

void QQuick3DSpecularGlossyMaterial::setAlbedoMap(QQuick3DTexture *albedoMap)
{
    if (m_albedoMap == albedoMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setAlbedoMap, albedoMap, m_albedoMap);

    m_albedoMap = albedoMap;
    emit albedoMapChanged();
    markDirty(AlbedoDirty);
}

void QQuick3DSpecularGlossyMaterial::setAlbedoSingleChannelEnabled(bool albedoSingleChannelEnabled)
{
    if (m_albedoSingleChannelEnabled == albedoSingleChannelEnabled)
        return;

    m_albedoSingleChannelEnabled = albedoSingleChannelEnabled;
    emit albedoSingleChannelEnabledChanged();
    markDirty(AlbedoDirty);
}

void QQuick3DSpecularGlossyMaterial::setAlbedoChannel(TextureChannelMapping channel)
{
    if (m_albedoChannel == channel)
        return;

    m_albedoChannel = channel;
    emit albedoChannelChanged();
    markDirty(AlbedoDirty);
}

void QQuick3DSpecularGlossyMaterial::setSpecularSingleChannelEnabled(bool specularSingleChannelEnabled)
{
    if (m_specularSingleChannelEnabled == specularSingleChannelEnabled)
        return;

    m_specularSingleChannelEnabled = specularSingleChannelEnabled;
    emit specularSingleChannelEnabledChanged();
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setSpecularChannel(TextureChannelMapping channel)
{
    if (m_specularChannel == channel)
        return;

    m_specularChannel = channel;
    emit specularChannelChanged();
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setEmissiveSingleChannelEnabled(bool emissiveSingleChannelEnabled)
{
    if (m_emissiveSingleChannelEnabled == emissiveSingleChannelEnabled)
        return;

    m_emissiveSingleChannelEnabled = emissiveSingleChannelEnabled;
    emit emissiveSingleChannelEnabledChanged();
    markDirty(EmissiveDirty);
}

void QQuick3DSpecularGlossyMaterial::setEmissiveChannel(TextureChannelMapping channel)
{
    if (m_emissiveChannel == channel)
        return;

    m_emissiveChannel = channel;
    emit emissiveChannelChanged();
    markDirty(EmissiveDirty);
}

void QQuick3DSpecularGlossyMaterial::setEmissiveMap(QQuick3DTexture *emissiveMap)
{
    if (m_emissiveMap == emissiveMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setEmissiveMap, emissiveMap, m_emissiveMap);

    m_emissiveMap = emissiveMap;
    emit emissiveMapChanged();
    markDirty(EmissiveDirty);
}

void QQuick3DSpecularGlossyMaterial::setEmissiveFactor(const QVector3D &emissiveFactor)
{
    if (m_emissiveFactor == emissiveFactor)
        return;

    m_emissiveFactor = emissiveFactor;
    emit emissiveFactorChanged();
    markDirty(EmissiveDirty);
}

void QQuick3DSpecularGlossyMaterial::setGlossiness(float glossiness)
{
    glossiness = ensureNormalized(glossiness);
    if (qFuzzyCompare(m_glossiness, glossiness))
        return;

    m_glossiness = glossiness;
    emit glossinessChanged();
    markDirty(GlossyDirty);
}

void QQuick3DSpecularGlossyMaterial::setGlossinessMap(QQuick3DTexture *glossinessMap)
{
    if (m_glossinessMap == glossinessMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setGlossinessMap, glossinessMap, m_glossinessMap);

    m_glossinessMap = glossinessMap;
    emit glossinessMapChanged();
    markDirty(GlossyDirty);
}

void QQuick3DSpecularGlossyMaterial::setInvertOpacityMapValue(bool invertOpacityMapValue)
{
    if (invertOpacityMapValue == m_invertOpacityMapValue)
        return;

    m_invertOpacityMapValue = invertOpacityMapValue;
    emit invertOpacityMapValueChanged();
    markDirty(OpacityDirty);
}

void QQuick3DSpecularGlossyMaterial::setOpacity(float opacity)
{
    opacity = ensureNormalized(opacity);
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    emit opacityChanged();
    markDirty(OpacityDirty);
}

void QQuick3DSpecularGlossyMaterial::setOpacityMap(QQuick3DTexture *opacityMap)
{
    if (m_opacityMap == opacityMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setOpacityMap, opacityMap, m_opacityMap);

    m_opacityMap = opacityMap;
    emit opacityMapChanged();
    markDirty(OpacityDirty);
}

void QQuick3DSpecularGlossyMaterial::setNormalMap(QQuick3DTexture *normalMap)
{
    if (m_normalMap == normalMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setNormalMap, normalMap, m_normalMap);

    m_normalMap = normalMap;
    emit normalMapChanged();
    markDirty(NormalDirty);
}

void QQuick3DSpecularGlossyMaterial::setSpecularColor(const QColor &specular)
{
    if (m_specular == specular)
        return;

    m_specular = specular;
    emit specularColorChanged();
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setSpecularMap(QQuick3DTexture *specularMap)
{
    if (m_specularMap == specularMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setSpecularMap, specularMap, m_specularMap);

    m_specularMap = specularMap;
    emit specularMapChanged();
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setNormalStrength(float factor)
{
    factor = ensureNormalized(factor);
    if (qFuzzyCompare(m_normalStrength, factor))
        return;

    m_normalStrength = factor;
    emit normalStrengthChanged();
    markDirty(NormalDirty);
}

void QQuick3DSpecularGlossyMaterial::setOcclusionMap(QQuick3DTexture *occlusionMap)
{
    if (m_occlusionMap == occlusionMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setOcclusionMap, occlusionMap, m_occlusionMap);

    m_occlusionMap = occlusionMap;
    emit occlusionMapChanged();
    markDirty(OcclusionDirty);
}

void QQuick3DSpecularGlossyMaterial::setOcclusionAmount(float occlusionAmount)
{
    if (qFuzzyCompare(m_occlusionAmount, occlusionAmount))
        return;

    m_occlusionAmount = occlusionAmount;
    emit occlusionAmountChanged();
    markDirty(OcclusionDirty);
}

void QQuick3DSpecularGlossyMaterial::setAlphaMode(QQuick3DSpecularGlossyMaterial::AlphaMode alphaMode)
{
    if (m_alphaMode == alphaMode)
        return;

    m_alphaMode = alphaMode;
    emit alphaModeChanged();
    markDirty(AlphaModeDirty);
}

void QQuick3DSpecularGlossyMaterial::setAlphaCutoff(float alphaCutoff)
{
    if (qFuzzyCompare(m_alphaCutoff, alphaCutoff))
        return;

    m_alphaCutoff = alphaCutoff;
    emit alphaCutoffChanged();
    markDirty(AlphaModeDirty);
}

void QQuick3DSpecularGlossyMaterial::setGlossinessChannel(TextureChannelMapping channel)
{
    if (m_glossinessChannel == channel)
        return;

    m_glossinessChannel = channel;
    emit glossinessChannelChanged();
    markDirty(GlossyDirty);
}

void QQuick3DSpecularGlossyMaterial::setOpacityChannel(TextureChannelMapping channel)
{
    if (m_opacityChannel == channel)
        return;

    m_opacityChannel = channel;
    emit opacityChannelChanged();
    markDirty(OpacityDirty);
}

void QQuick3DSpecularGlossyMaterial::setOcclusionChannel(TextureChannelMapping channel)
{
    if (m_occlusionChannel == channel)
        return;

    m_occlusionChannel = channel;
    emit occlusionChannelChanged();
    markDirty(OcclusionDirty);
}

void QQuick3DSpecularGlossyMaterial::setPointSize(float size)
{
    if (qFuzzyCompare(m_pointSize, size))
        return;
    m_pointSize = size;
    emit pointSizeChanged();
    markDirty(PointSizeDirty);
}

void QQuick3DSpecularGlossyMaterial::setLineWidth(float width)
{
    if (qFuzzyCompare(m_lineWidth, width))
        return;
    m_lineWidth = width;
    emit lineWidthChanged();
    markDirty(LineWidthDirty);
}

void QQuick3DSpecularGlossyMaterial::setHeightMap(QQuick3DTexture *heightMap)
{
    if (m_heightMap == heightMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setHeightMap, heightMap, m_heightMap);

    m_heightMap = heightMap;
    emit heightMapChanged();
    markDirty(HeightDirty);
}

void QQuick3DSpecularGlossyMaterial::setHeightChannel(QQuick3DMaterial::TextureChannelMapping channel)
{
    if (m_heightChannel == channel)
        return;

    m_heightChannel = channel;
    emit heightChannelChanged();
    markDirty(HeightDirty);
}

void QQuick3DSpecularGlossyMaterial::setHeightAmount(float heightAmount)
{
    if (qFuzzyCompare(m_heightAmount, heightAmount))
        return;

    m_heightAmount = heightAmount;
    emit heightAmountChanged();
    markDirty(HeightDirty);
}

void QQuick3DSpecularGlossyMaterial::setMinHeightMapSamples(int samples)
{
    if (m_minHeightMapSamples == samples)
        return;

    m_minHeightMapSamples = samples;
    emit minHeightMapSamplesChanged();
    markDirty(HeightDirty);
}

void QQuick3DSpecularGlossyMaterial::setMaxHeightMapSamples(int samples)
{
    if (m_maxHeightMapSamples == samples)
        return;

    m_maxHeightMapSamples = samples;
    emit maxHeightMapSamplesChanged();
    markDirty(HeightDirty);
}

QSSGRenderGraphObject *QQuick3DSpecularGlossyMaterial::updateSpatialNode(QSSGRenderGraphObject *node)
{
    static const auto channelMapping = [](TextureChannelMapping mapping) {
        return QSSGRenderDefaultMaterial::TextureChannelMapping(mapping);
    };

    if (!node) {
        markAllDirty();
        node = new QSSGRenderDefaultMaterial(QSSGRenderGraphObject::Type::SpecularGlossyMaterial);
    }

    // Set common material properties
    QQuick3DMaterial::updateSpatialNode(node);

    QSSGRenderDefaultMaterial *material = static_cast<QSSGRenderDefaultMaterial *>(node);

    material->specularModel = QSSGRenderDefaultMaterial::MaterialSpecularModel::SchlickGGX;

    if (m_dirtyAttributes & LightingModeDirty)
        material->lighting = QSSGRenderDefaultMaterial::MaterialLighting(m_lighting);

    if (m_dirtyAttributes & BlendModeDirty)
        material->blendMode = QSSGRenderDefaultMaterial::MaterialBlendMode(m_blendMode);

    if (m_dirtyAttributes & AlbedoDirty) {
        if (!m_albedoMap)
            material->colorMap = nullptr;
        else
            material->colorMap = m_albedoMap->getRenderImage();

        material->color = QSSGUtils::color::sRGBToLinear(m_albedo);
        material->baseColorSingleChannelEnabled = m_albedoSingleChannelEnabled;
        material->baseColorChannel = channelMapping(m_albedoChannel);
    }

    if (m_dirtyAttributes & EmissiveDirty) {
        if (!m_emissiveMap)
            material->emissiveMap = nullptr;
        else
            material->emissiveMap = m_emissiveMap->getRenderImage();

        material->emissiveColor = m_emissiveFactor;
        material->emissiveSingleChannelEnabled = m_emissiveSingleChannelEnabled;
        material->emissiveChannel = channelMapping(m_emissiveChannel);
    }

    if (m_dirtyAttributes & GlossyDirty) {
        if (!m_glossinessMap)
            material->roughnessMap = nullptr;
        else
            material->roughnessMap = m_glossinessMap->getRenderImage();

        material->specularRoughness = m_glossiness;
        material->roughnessChannel = channelMapping(m_glossinessChannel);
    }

    if (m_dirtyAttributes & SpecularDirty) {
        if (!m_specularMap)
            material->specularMap = nullptr;
        else
            material->specularMap = m_specularMap->getRenderImage();

        material->specularTint = QSSGUtils::color::sRGBToLinear(m_specular).toVector3D();
        material->fresnelScaleBiasEnabled = m_fresnelScaleBiasEnabled;
        material->fresnelScale = m_fresnelScale;
        material->fresnelBias = m_fresnelBias;
        material->fresnelPower = m_fresnelPower;
        material->specularAmountSingleChannelEnabled = m_specularSingleChannelEnabled;
        material->specularAmountChannel = channelMapping(m_specularChannel);
    }

    if (m_dirtyAttributes & OpacityDirty) {
        if (!m_opacityMap)
            material->opacityMap = nullptr;
        else
            material->opacityMap = m_opacityMap->getRenderImage();

        material->invertOpacityMapValue = m_invertOpacityMapValue;
        material->opacity = m_opacity;
        material->opacityChannel = channelMapping(m_opacityChannel);
    }

    if (m_dirtyAttributes & NormalDirty) {
        if (!m_normalMap)
            material->normalMap = nullptr;
        else
            material->normalMap = m_normalMap->getRenderImage();

        material->bumpAmount = m_normalStrength;
    }

    if (m_dirtyAttributes & OcclusionDirty) {
        if (!m_occlusionMap)
            material->occlusionMap = nullptr;
        else
            material->occlusionMap = m_occlusionMap->getRenderImage();
        material->occlusionAmount = m_occlusionAmount;
        material->occlusionChannel = channelMapping(m_occlusionChannel);
    }

    if (m_dirtyAttributes & AlphaModeDirty) {
        material->alphaMode = QSSGRenderDefaultMaterial::MaterialAlphaMode(m_alphaMode);
        material->alphaCutoff = m_alphaCutoff;
    }

    if (m_dirtyAttributes & PointSizeDirty)
        material->pointSize = m_pointSize;

    if (m_dirtyAttributes & LineWidthDirty)
        material->lineWidth = m_lineWidth;

    if (m_dirtyAttributes & HeightDirty) {
        if (!m_heightMap)
            material->heightMap = nullptr;
        else
            material->heightMap = m_heightMap->getRenderImage();
        material->heightAmount = m_heightAmount;
        material->minHeightSamples = m_minHeightMapSamples;
        material->maxHeightSamples = m_maxHeightMapSamples;
        material->heightChannel = channelMapping(m_heightChannel);
    }

    if (m_dirtyAttributes & ClearcoatDirty) {
        material->clearcoatAmount = m_clearcoatAmount;
        if (!m_clearcoatMap)
            material->clearcoatMap = nullptr;
        else
            material->clearcoatMap = m_clearcoatMap->getRenderImage();
        material->clearcoatChannel = channelMapping(m_clearcoatChannel);
        material->clearcoatRoughnessAmount = m_clearcoatRoughnessAmount;
        if (!m_clearcoatRoughnessMap)
            material->clearcoatRoughnessMap = nullptr;
        else
            material->clearcoatRoughnessMap = m_clearcoatRoughnessMap->getRenderImage();
        material->clearcoatRoughnessChannel = channelMapping(m_clearcoatRoughnessChannel);
        if (!m_clearcoatNormalMap)
            material->clearcoatNormalMap = nullptr;
        else
            material->clearcoatNormalMap = m_clearcoatNormalMap->getRenderImage();
        material->clearcoatNormalStrength = m_clearcoatNormalStrength;
        material->clearcoatFresnelScaleBiasEnabled = m_clearcoatFresnelScaleBiasEnabled;
        material->clearcoatFresnelScale = m_clearcoatFresnelScale;
        material->clearcoatFresnelBias = m_clearcoatFresnelBias;
        material->clearcoatFresnelPower = m_clearcoatFresnelPower;
    }

    if (m_dirtyAttributes & TransmissionDirty) {
        material->transmissionFactor = m_transmissionFactor;
        if (!m_transmissionMap)
            material->transmissionMap = nullptr;
        else
            material->transmissionMap = m_transmissionMap->getRenderImage();
        material->transmissionChannel = channelMapping(m_transmissionChannel);
    }

    if (m_dirtyAttributes & VolumeDirty) {
        material->thicknessFactor = m_thicknessFactor;
        if (!m_thicknessMap)
            material->thicknessMap = nullptr;
        else
            material->thicknessMap = m_thicknessMap->getRenderImage();
        material->thicknessChannel = channelMapping(m_thicknessChannel);

        material->attenuationDistance = m_attenuationDistance;
        material->attenuationColor = QSSGUtils::color::sRGBToLinear(m_attenuationColor).toVector3D();
    }

    if (m_dirtyAttributes & VertexColorsDirty) {
        material->vertexColorsEnabled = m_vertexColorsEnabled;
        material->vertexColorsMaskEnabled = m_vertexColorsMaskEnabled;
        material->vertexColorRedMask = QSSGRenderDefaultMaterial::VertexColorMaskFlags::fromInt(m_vertexColorRedMask);
        material->vertexColorGreenMask = QSSGRenderDefaultMaterial::VertexColorMaskFlags::fromInt(m_vertexColorGreenMask);
        material->vertexColorBlueMask = QSSGRenderDefaultMaterial::VertexColorMaskFlags::fromInt(m_vertexColorBlueMask);
        material->vertexColorAlphaMask = QSSGRenderDefaultMaterial::VertexColorMaskFlags::fromInt(m_vertexColorAlphaMask);
    }

    m_dirtyAttributes = 0;

    return node;
}

void QQuick3DSpecularGlossyMaterial::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

void QQuick3DSpecularGlossyMaterial::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    // Check all the resource value's scene manager, and update as necessary.
    if (sceneManager) {
        QQuick3DObjectPrivate::refSceneManager(m_albedoMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_emissiveMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_glossinessMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_opacityMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_normalMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_specularMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_occlusionMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_heightMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_clearcoatMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_clearcoatRoughnessMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_clearcoatNormalMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_transmissionMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_thicknessMap, *sceneManager);
    } else {
        QQuick3DObjectPrivate::derefSceneManager(m_albedoMap);
        QQuick3DObjectPrivate::derefSceneManager(m_emissiveMap);
        QQuick3DObjectPrivate::derefSceneManager(m_glossinessMap);
        QQuick3DObjectPrivate::derefSceneManager(m_opacityMap);
        QQuick3DObjectPrivate::derefSceneManager(m_normalMap);
        QQuick3DObjectPrivate::derefSceneManager(m_specularMap);
        QQuick3DObjectPrivate::derefSceneManager(m_occlusionMap);
        QQuick3DObjectPrivate::derefSceneManager(m_heightMap);
        QQuick3DObjectPrivate::derefSceneManager(m_clearcoatMap);
        QQuick3DObjectPrivate::derefSceneManager(m_clearcoatRoughnessMap);
        QQuick3DObjectPrivate::derefSceneManager(m_clearcoatNormalMap);
        QQuick3DObjectPrivate::derefSceneManager(m_transmissionMap);
        QQuick3DObjectPrivate::derefSceneManager(m_thicknessMap);
    }
}

void QQuick3DSpecularGlossyMaterial::markDirty(QQuick3DSpecularGlossyMaterial::DirtyType type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

float QQuick3DSpecularGlossyMaterial::clearcoatAmount() const
{
    return m_clearcoatAmount;
}

void QQuick3DSpecularGlossyMaterial::setClearcoatAmount(float newClearcoatAmount)
{
    if (qFuzzyCompare(m_clearcoatAmount, newClearcoatAmount))
        return;
    m_clearcoatAmount = newClearcoatAmount;
    emit clearcoatAmountChanged();
    markDirty(ClearcoatDirty);
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::clearcoatMap() const
{
    return m_clearcoatMap;
}

void QQuick3DSpecularGlossyMaterial::setClearcoatMap(QQuick3DTexture *newClearcoatMap)
{
    if (m_clearcoatMap == newClearcoatMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setClearcoatMap, newClearcoatMap, m_clearcoatMap);

    m_clearcoatMap = newClearcoatMap;
    emit clearcoatMapChanged();
    markDirty(ClearcoatDirty);
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::clearcoatChannel() const
{
    return m_clearcoatChannel;
}

void QQuick3DSpecularGlossyMaterial::setClearcoatChannel(QQuick3DMaterial::TextureChannelMapping newClearcoatChannel)
{
    if (m_clearcoatChannel == newClearcoatChannel)
        return;
    m_clearcoatChannel = newClearcoatChannel;
    emit clearcoatChannelChanged();
    markDirty(ClearcoatDirty);
}

float QQuick3DSpecularGlossyMaterial::clearcoatRoughnessAmount() const
{
    return m_clearcoatRoughnessAmount;
}

void QQuick3DSpecularGlossyMaterial::setClearcoatRoughnessAmount(float newClearcoatRoughnessAmount)
{
    if (qFuzzyCompare(m_clearcoatRoughnessAmount, newClearcoatRoughnessAmount))
        return;
    m_clearcoatRoughnessAmount = newClearcoatRoughnessAmount;
    emit clearcoatRoughnessAmountChanged();
    markDirty(ClearcoatDirty);
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::clearcoatRoughnessChannel() const
{
    return m_clearcoatRoughnessChannel;
}

void QQuick3DSpecularGlossyMaterial::setClearcoatRoughnessChannel(QQuick3DMaterial::TextureChannelMapping newClearcoatRoughnessChannel)
{
    if (m_clearcoatRoughnessChannel == newClearcoatRoughnessChannel)
        return;
    m_clearcoatRoughnessChannel = newClearcoatRoughnessChannel;
    emit clearcoatRoughnessChannelChanged();
    markDirty(ClearcoatDirty);
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::clearcoatRoughnessMap() const
{
    return m_clearcoatRoughnessMap;
}

void QQuick3DSpecularGlossyMaterial::setClearcoatRoughnessMap(QQuick3DTexture *newClearcoatRoughnessMap)
{
    if (m_clearcoatRoughnessMap == newClearcoatRoughnessMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setClearcoatRoughnessMap, newClearcoatRoughnessMap, m_clearcoatRoughnessMap);

    m_clearcoatRoughnessMap = newClearcoatRoughnessMap;
    emit clearcoatRoughnessMapChanged();
    markDirty(ClearcoatDirty);
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::clearcoatNormalMap() const
{
    return m_clearcoatNormalMap;
}

void QQuick3DSpecularGlossyMaterial::setClearcoatNormalMap(QQuick3DTexture *newClearcoatNormalMap)
{
    if (m_clearcoatNormalMap == newClearcoatNormalMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setClearcoatNormalMap, newClearcoatNormalMap, m_clearcoatNormalMap);

    m_clearcoatNormalMap = newClearcoatNormalMap;
    emit clearcoatNormalMapChanged();
    markDirty(ClearcoatDirty);
}

float QQuick3DSpecularGlossyMaterial::clearcoatNormalStrength() const
{
    return m_clearcoatNormalStrength;
}


void QQuick3DSpecularGlossyMaterial::setClearcoatNormalStrength(float newClearcoatNormalStrength)
{
    if (qFuzzyCompare(m_clearcoatNormalStrength, newClearcoatNormalStrength))
        return;
    m_clearcoatNormalStrength = newClearcoatNormalStrength;
    emit clearcoatNormalStrengthChanged();
    markDirty(ClearcoatDirty);
}

float QQuick3DSpecularGlossyMaterial::transmissionFactor() const
{
    return m_transmissionFactor;
}

void QQuick3DSpecularGlossyMaterial::setTransmissionFactor(float newTransmissionFactor)
{
    if (qFuzzyCompare(m_transmissionFactor, newTransmissionFactor))
        return;
    m_transmissionFactor = newTransmissionFactor;
    emit transmissionFactorChanged();
    markDirty(TransmissionDirty);
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::transmissionMap() const
{
    return m_transmissionMap;
}

void QQuick3DSpecularGlossyMaterial::setTransmissionMap(QQuick3DTexture *newTransmissionMap)
{
    if (m_transmissionMap == newTransmissionMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setTransmissionMap, newTransmissionMap, m_transmissionMap);

    m_transmissionMap = newTransmissionMap;
    emit transmissionMapChanged();
    markDirty(TransmissionDirty);
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::transmissionChannel() const
{
    return m_transmissionChannel;
}

void QQuick3DSpecularGlossyMaterial::setTransmissionChannel(QQuick3DMaterial::TextureChannelMapping newTransmissionChannel)
{
    if (m_transmissionChannel == newTransmissionChannel)
        return;
    m_transmissionChannel = newTransmissionChannel;
    emit transmissionChannelChanged();
    markDirty(TransmissionDirty);
}

float QQuick3DSpecularGlossyMaterial::thicknessFactor() const
{
    return m_thicknessFactor;
}

void QQuick3DSpecularGlossyMaterial::setThicknessFactor(float newThicknessFactor)
{
    if (qFuzzyCompare(m_thicknessFactor, newThicknessFactor))
        return;
    m_thicknessFactor = newThicknessFactor;
    emit thicknessFactorChanged();
    markDirty(VolumeDirty);
}

QQuick3DTexture *QQuick3DSpecularGlossyMaterial::thicknessMap() const
{
    return m_thicknessMap;
}

void QQuick3DSpecularGlossyMaterial::setThicknessMap(QQuick3DTexture *newThicknessMap)
{
    if (m_thicknessMap == newThicknessMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DSpecularGlossyMaterial::setThicknessMap, newThicknessMap, m_thicknessMap);

    m_thicknessMap = newThicknessMap;
    emit thicknessMapChanged();
    markDirty(VolumeDirty);
}

QQuick3DMaterial::TextureChannelMapping QQuick3DSpecularGlossyMaterial::thicknessChannel() const
{
    return m_thicknessChannel;
}

void QQuick3DSpecularGlossyMaterial::setThicknessChannel(QQuick3DMaterial::TextureChannelMapping newThicknessChannel)
{
    if (m_thicknessChannel == newThicknessChannel)
        return;
    m_thicknessChannel = newThicknessChannel;
    emit thicknessChannelChanged();
    markDirty(VolumeDirty);
}

float QQuick3DSpecularGlossyMaterial::attenuationDistance() const
{
    return m_attenuationDistance;
}

void QQuick3DSpecularGlossyMaterial::setAttenuationDistance(float newAttenuationDistance)
{
    if (qFuzzyCompare(m_attenuationDistance, newAttenuationDistance))
        return;
    m_attenuationDistance = newAttenuationDistance;
    emit attenuationDistanceChanged();
    markDirty(VolumeDirty);
}

QColor QQuick3DSpecularGlossyMaterial::attenuationColor() const
{
    return m_attenuationColor;
}

bool QQuick3DSpecularGlossyMaterial::fresnelScaleBiasEnabled() const
{
    return m_fresnelScaleBiasEnabled;
}

float QQuick3DSpecularGlossyMaterial::fresnelScale() const
{
    return m_fresnelScale;
}

float QQuick3DSpecularGlossyMaterial::fresnelBias() const
{
    return m_fresnelBias;
}

float QQuick3DSpecularGlossyMaterial::fresnelPower() const
{
    return m_fresnelPower;
}

bool QQuick3DSpecularGlossyMaterial::clearcoatFresnelScaleBiasEnabled() const
{
    return m_clearcoatFresnelScaleBiasEnabled;
}

float QQuick3DSpecularGlossyMaterial::clearcoatFresnelScale() const
{
    return m_clearcoatFresnelScale;
}

float QQuick3DSpecularGlossyMaterial::clearcoatFresnelBias() const
{
    return m_clearcoatFresnelBias;
}

float QQuick3DSpecularGlossyMaterial::clearcoatFresnelPower() const
{
    return m_clearcoatFresnelPower;
}

bool QQuick3DSpecularGlossyMaterial::vertexColorsEnabled() const
{
    return m_vertexColorsEnabled;
}

void QQuick3DSpecularGlossyMaterial::setAttenuationColor(const QColor &newAttenuationColor)
{
    if (m_attenuationColor == newAttenuationColor)
        return;
    m_attenuationColor = newAttenuationColor;
    emit attenuationColorChanged();
    markDirty(VolumeDirty);
}

void QQuick3DSpecularGlossyMaterial::setFresnelScaleBiasEnabled(bool fresnelScaleBiasEnabled)
{
    if (m_fresnelScaleBiasEnabled == fresnelScaleBiasEnabled)
        return;

    m_fresnelScaleBiasEnabled = fresnelScaleBiasEnabled;
    emit fresnelScaleBiasEnabledChanged(m_fresnelScaleBiasEnabled);
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setFresnelScale(float fresnelScale)
{
    if (qFuzzyCompare(m_fresnelScale, fresnelScale))
        return;

    m_fresnelScale = fresnelScale;
    emit fresnelScaleChanged(m_fresnelScale);
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setFresnelBias(float fresnelBias)
{
    if (qFuzzyCompare(m_fresnelBias, fresnelBias))
        return;

    m_fresnelBias = fresnelBias;
    emit fresnelBiasChanged(m_fresnelBias);
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setFresnelPower(float fresnelPower)
{
    if (qFuzzyCompare(m_fresnelPower, fresnelPower))
        return;

    m_fresnelPower = fresnelPower;
    emit fresnelPowerChanged(m_fresnelPower);
    markDirty(SpecularDirty);
}

void QQuick3DSpecularGlossyMaterial::setClearcoatFresnelScaleBiasEnabled(bool clearcoatFresnelScaleBiasEnabled)
{
    if (m_clearcoatFresnelScaleBiasEnabled == clearcoatFresnelScaleBiasEnabled)
        return;

    m_clearcoatFresnelScaleBiasEnabled = clearcoatFresnelScaleBiasEnabled;
    emit clearcoatFresnelScaleBiasEnabledChanged(m_clearcoatFresnelScaleBiasEnabled);
    markDirty(ClearcoatDirty);
}

void QQuick3DSpecularGlossyMaterial::setClearcoatFresnelScale(float clearcoatFresnelScale)
{
    if (qFuzzyCompare(m_clearcoatFresnelScale, clearcoatFresnelScale))
        return;

    m_clearcoatFresnelScale = clearcoatFresnelScale;
    emit clearcoatFresnelScaleChanged(m_clearcoatFresnelScale);
    markDirty(ClearcoatDirty);
}

void QQuick3DSpecularGlossyMaterial::setClearcoatFresnelBias(float clearcoatFresnelBias)
{
    if (qFuzzyCompare(m_clearcoatFresnelBias, clearcoatFresnelBias))
        return;

    m_clearcoatFresnelBias = clearcoatFresnelBias;
    emit clearcoatFresnelBiasChanged(m_clearcoatFresnelBias);
    markDirty(ClearcoatDirty);
}

void QQuick3DSpecularGlossyMaterial::setClearcoatFresnelPower(float clearcoatFresnelPower)
{
    if (qFuzzyCompare(m_clearcoatFresnelPower, clearcoatFresnelPower))
        return;

    m_clearcoatFresnelPower = clearcoatFresnelPower;
    emit clearcoatFresnelPowerChanged(m_clearcoatFresnelPower);
    markDirty(ClearcoatDirty);
}

void QQuick3DSpecularGlossyMaterial::setVertexColorsEnabled(bool vertexColors)
{
    if (m_vertexColorsEnabled == vertexColors)
        return;

    m_vertexColorsEnabled = vertexColors;
    emit vertexColorsEnabledChanged(m_vertexColorsEnabled);
    markDirty(VertexColorsDirty);
}

bool QQuick3DSpecularGlossyMaterial::vertexColorsMaskEnabled() const
{
    return m_vertexColorsMaskEnabled;
}

void QQuick3DSpecularGlossyMaterial::setVertexColorsMaskEnabled(bool vertexColorsMaskEnabled)
{
    if (m_vertexColorsMaskEnabled == vertexColorsMaskEnabled)
        return;
    m_vertexColorsMaskEnabled = vertexColorsMaskEnabled;
    emit vertexColorsMaskEnabledChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags QQuick3DSpecularGlossyMaterial::vertexColorRedMask() const
{
    return m_vertexColorRedMask;
}

void QQuick3DSpecularGlossyMaterial::setVertexColorRedMask(QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags vertexColorRedMask)
{
    if (m_vertexColorRedMask == vertexColorRedMask)
        return;
    m_vertexColorRedMask = vertexColorRedMask;
    emit vertexColorRedMaskChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags QQuick3DSpecularGlossyMaterial::vertexColorGreenMask() const
{
    return m_vertexColorGreenMask;
}

void QQuick3DSpecularGlossyMaterial::setVertexColorGreenMask(QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags vertexColorGreenMask)
{
    if (m_vertexColorGreenMask == vertexColorGreenMask)
        return;
    m_vertexColorGreenMask = vertexColorGreenMask;
    emit vertexColorGreenMaskChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags QQuick3DSpecularGlossyMaterial::vertexColorBlueMask() const
{
    return m_vertexColorBlueMask;
}

void QQuick3DSpecularGlossyMaterial::setVertexColorBlueMask(QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags vertexColorBlueMask)
{
    if (m_vertexColorBlueMask == vertexColorBlueMask)
        return;
    m_vertexColorBlueMask = vertexColorBlueMask;
    emit vertexColorBlueMaskChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags QQuick3DSpecularGlossyMaterial::vertexColorAlphaMask() const
{
    return m_vertexColorAlphaMask;
}

void QQuick3DSpecularGlossyMaterial::setVertexColorAlphaMask(QQuick3DSpecularGlossyMaterial::VertexColorMaskFlags vertexColorAlphaMask)
{
    if (m_vertexColorAlphaMask == vertexColorAlphaMask)
        return;
    m_vertexColorAlphaMask = vertexColorAlphaMask;
    emit vertexColorAlphaMaskChanged();
    markDirty(VertexColorsDirty);
}

QT_END_NAMESPACE

// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dprincipledmaterial_p.h"
#include "qquick3dobject_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrenderdefaultmaterial_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype PrincipledMaterial
    \inherits Material
    \inqmlmodule QtQuick3D
    \brief Lets you define a material for 3D items using the metal/roughness workflow.

    Before a Model can be rendered in a scene, it must have at least one material attached
    to it that describes how the mesh should be shaded. The PrincipledMaterial is a PBR
    metal/roughness material that aims at being an easy to use material with a minimal
    set of parameters.
    In addition to having few parameters, all input values are strictly normalized
    between 0 and 1 and have sensible defaults, meaning even without changing any values,
    the material can be used to shader a model. For an introduction on how the
    different properties of the principled material affects how a model is shaded,
    see the \l{Qt Quick 3D - Principled Material Example}{Principled Material example}.

    \section1 Metal/Roughness workflow

    The Principled material is what's known as a metal/roughness material, in essence
    that means the main characteristics of the material is controlled through
    the \l {PrincipledMaterial::metalnessMap} {metallness}, \l {PrincipledMaterial::roughnessMap} {roughness},
    and the \l {PrincipledMaterial::baseColorMap} {base color} property.

    \section2 Metalness

    Real world materials are put into two main categories, metals and dielectrics (non-metals).
    In the Principled material, the category a material belongs to is decided by the
    \c metalness value. Setting the \c metalness value to 0, means the material is a dialectric,
    while everything above 0 is a considered to be a metal. In reality metals will have
    a \c metalness value of 1, but values between 0 and 1 are possible, and usually used
    for metals with reduced reflectance. For example, to render corrosion, or similar,
    on a material, the \c metalness of the material should be reduced to give the output
    properties more similar to a dielectric material.
    Since the \c metalness value affects the reflectance of the material it might be tempting to
    use the metalness to adjust glossiness, but consider what type of material you want
    to describe first. Increasing the \c metalness value to give a dielectric material
    a more polished look, will introduce properties that are not accurate for a dielectric
    material, so consider if it would be more appropriate to adjust, for example,
    the \c roughness value instead.

    \section2 Roughness

    The \c roughness of a material describes the condition of an object's surface.
    A low \c roughness value means the object has a smooth surface and therefore be more
    reflective then a material with a higher \c roughness value.

    \section2 Base color

    The \l {PrincipledMaterial::baseColorMap} {base color} of a metal/roughness material
    contains both the diffuse and the specular data, how much the base color is interpreted
    as one or the other is primarily dictated by the \c metalness value. For example,
    a material with a metalness value of 1, will have most of its base color interpreted
    as specular color, while the diffuse color would be a black tint. The opposite would
    happen for a material with a metalness value of 0. This is of course a bit simplified,
    but gives a rough idea how the \l {PrincipledMaterial::baseColor} {base color} and
    \c metalness value interacts. For those more familiar with a Specular/Glossiness workflow,
    there's a clear difference here which is worth noting, namely that the color data of the
    two materials are not directly compatible, since in a Specular/Glossiness
    \l {DefaultMaterial} {material}, the diffuse and specular color comes from separate inputs.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::lighting

    This property defines which lighting method is used when generating this
    material.

    The default value is \c PrincipledMaterial.FragmentLighting

    When using \c PrincipledMaterial.FragmentLighting, diffuse and specular lighting is
    calculated for each rendered pixel. Certain effects (such as a Fresnel or normal map) require
    \c PrincipledMaterial.FragmentLighting to work.

    When using \c PrincipledMaterial.NoLighting no lighting is calculated. This
    mode is (predictably) very fast, and is quite effective when image maps are
    used that you do not need to be shaded by lighting. All other shading
    properties except baseColor values, alpha values, and vertex colors will be
    ignored.

    \value PrincipledMaterial.NoLighting
    \value PrincipledMaterial.FragmentLighting
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::blendMode

    This property determines how the colors of the model rendered blends with
    those behind it.

    \value PrincipledMaterial.SourceOver Default blend mode. Opaque objects
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

    \value PrincipledMaterial.Screen Colors are blended using an inverted
    multiply, producing a lighter result. This blend mode is order-independent;
    if you are using semi-opaque objects and experiencing 'popping' as faces or
    models sort differently, using Screen blending is one way to produce
    results without popping.

    \value PrincipledMaterial.Multiply Colors are blended using a multiply,
    producing a darker result. This blend mode is also order-independent.

    \sa alphaMode, {Qt Quick 3D Architecture}
*/

/*!
    \qmlproperty color PrincipledMaterial::baseColor

    This property sets the base color for the material. Depending on the type
    of material specified (metal or dielectric) the diffuse and specular channels will be
    set appropriately. For example, a dielectric material will have a diffuse color equal to
    the base color, while it's specular color, depending on the specular amount, will have a
    bright specular color. For metals the diffuse and specular channels will be mixed from
    the base color and have a dark diffuse channel and a specular channel close to the base color.

    \sa baseColorMap, alphaMode
*/

/*!
    \qmlproperty Texture PrincipledMaterial::baseColorMap

    This property defines the texture used to set the base color of the material.

    \sa baseColor, alphaMode
*/

/*!
    \qmlproperty bool PrincipledMaterial::baseColorSingleChannelEnabled
    \since 6.8

    When this property is enabled, the material will use the single value of the baseColorChannel from
    the baseColorMap as RGB value and use 1.0 as alpha value.
    The default value is false.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::baseColorChannel
    \since 6.8

    This property defines the texture channel used to read the baseColor value from baseColorMap.
    In order to use a single texture channel as color you have to enable the baseColorSingleChannelEnabled
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty real PrincipledMaterial::metalness

    The metalness property defines the \e metalness of the the material. The value
    is normalized, where 0.0 means the material is a \e dielectric (non-metallic) material and
    a value of 1.0 means the material is a metal.

    \note In principle, materials are either dielectrics with a metalness of 0, or metals with a
    metalness of 1. Metalness values between 0 and 1 are still allowed and will give a material that
    is a blend between the different models.

    The range is [0.0, 1.0]. The default value is 0.
*/

/*!
    \qmlproperty Texture PrincipledMaterial::metalnessMap

    This property sets a Texture to be used to set the metalness amount for the
    different parts of the material.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::metalnessChannel

    This property defines the texture channel used to read the metalness value from metalnessMap.
    The default value is \c Material.B.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty bool PrincipledMaterial::emissiveSingleChannelEnabled
    \since 6.8

    When this property is enabled, the material will use the single value of the emissiveChannel from
    the emissiveMap as RGB value.
    The default value is false.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::emissiveChannel
    \since 6.8

    This property defines the texture channel used to read the emissive value from emissiveMap.
    In order to use a single texture channel as color you have to enable the emissiveSingleChannelEnabled
    The default value is \c Material.R.

     \value Material.R Read value from texture R channel.
     \value Material.G Read value from texture G channel.
     \value Material.B Read value from texture B channel.
     \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty Texture PrincipledMaterial::emissiveMap

    This property sets a RGB Texture to be used to specify the intensity of the
    emissive color.
*/

/*!
    \qmlproperty vector3d PrincipledMaterial::emissiveFactor

    This property determines the color of self-illumination for this material.
    If an emissive map is set, the x, y, and z components are used as factors
    (multipliers) for the R, G and B channels of the texture, respectively.
    The default value is (0, 0, 0) and it means no emissive contribution at all.

    \note Setting the lightingMode to DefaultMaterial.NoLighting means emissive
    Factor does not have an effect on the scene.
*/

/*!
    \qmlproperty Texture PrincipledMaterial::specularReflectionMap

    This property sets a Texture used for specular highlights on the material.

    This is typically used to perform environment mapping: as the model is
    rotated, the map will appear as though it is reflecting from the
    environment. For this to work as expected, the Texture's
    \l{Texture::mappingMode}{mappingMode} needs to be set to
    Texture.Environment. Specular reflection maps are an easy way to add a
    high-quality look with a relatively low cost.

    \note Associating a \l{SceneEnvironment::lightProbe}{light probe} with the
    \l SceneEnvironment, and thus relying on image-based lighting, can achieve
    similar environmental reflection effects. Light probes are however a
    conceptually different, and when it comes to performance, potentially more
    expensive solution. Each approaches have their own specific uses, and the
    one to use needs to be decided on a case by case basis. When it comes to
    the Texture set to the property, specularReflectionMap has an advantage,
    because it presents no limitations and supports all types of textures,
    including ones that source their data from a Qt Quick sub-scene via
    \l{Texture::sourceItem}{sourceItem}.

    \note Crisp images cause your material to look very glossy; the more you
    blur your image the softer your material will appear.

    \sa Texture::mappingMode
*/

/*!
    \qmlproperty Texture PrincipledMaterial::specularMap

    The property defines a RGB Texture to modulate the amount and the color of
    specularity across the surface of the material. These values are multiplied
    by the specularAmount.

    \note The specular map will be ignored unless the material is dielectric.
*/

/*!
    \qmlproperty bool PrincipledMaterial::specularSingleChannelEnabled
    \since 6.8

    When this property is enabled, the material will use the single value of the specularChannel from
    the specularMap as RGB value.
    The default value is false.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::specularChannel
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
    \qmlproperty real PrincipledMaterial::specularTint

    This property defines how much of the base color contributes to the specular reflections.

    \note This property does only apply to dielectric materials.
*/

/*!
    \qmlproperty real PrincipledMaterial::specularAmount

    This property controls the strength of specularity (highlights and
    reflections).

    The range is [0.0, 1.0]. The default value is \c 1.0.

    \note For non-dielectrics (metals) this property has no effect.

    \note This property does not affect the specularReflectionMap, but does affect the amount of
    reflections from a scenes SceneEnvironment::lightProbe.
*/

/*!
    \qmlproperty real PrincipledMaterial::roughness

    This property controls the size of the specular highlight generated from
    lights, and the clarity of reflections in general. Larger values increase
    the roughness, softening specular highlights and blurring reflections.
    The range is [0.0, 1.0]. The default value is 0.
*/

/*!
    \qmlproperty Texture PrincipledMaterial::roughnessMap

    This property defines a Texture to control the specular roughness of the
    material.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::roughnessChannel

    This property defines the texture channel used to read the roughness value from roughnessMap.
    The default value is \c Material.G.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty real PrincipledMaterial::opacity

    This property drops the opacity of just this material, separate from the
    model.
*/

/*!
    \qmlproperty Texture PrincipledMaterial::opacityMap

    This property defines a Texture used to control the opacity differently for
    different parts of the material.
*/

/*!
    \qmlproperty real PrincipledMaterial::invertOpacityMapValue
    \since 6.8

    This property inverts the opacity value of the opacityMap.
    The default value is \c false.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::opacityChannel

    This property defines the texture channel used to read the opacity value from opacityMap.
    The default value is \c Material.A.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty Texture PrincipledMaterial::normalMap

    This property defines an RGB image used to simulate fine geometry
    displacement across the surface of the material. The RGB channels indicate
    XYZ normal deviations.

    \note Normal maps will not affect the silhouette of a model.
*/

/*!
    \qmlproperty real PrincipledMaterial::normalStrength

    This property controls the amount of simulated displacement for the normalMap.
*/

/*!
    \qmlproperty real PrincipledMaterial::occlusionAmount

    This property contains the factor used to modify the values from the \l occlusionMap texture.
    The value should be between 0.0 to 1.0. The default is 1.0
*/

/*!
    \qmlproperty Texture PrincipledMaterial::occlusionMap

    This property defines a texture used to determine how much light the
    different areas of the material should receive. Values are expected to be
    linear from 0.0 to 1.0, where 0.0 means no lighting and 1.0 means the
    effect of the lighting is left unchanged.

    \sa occlusionAmount
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::occlusionChannel

    This property defines the texture channel used to read the occlusion value from occlusionMap.
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::alphaMode

    This property specifies how the alpha color value from \l baseColor and the
    alpha channel of a \l{baseColorMap}{base color map} are used.

    \note The alpha cutoff test only considers the base color alpha. \l opacity
    and \l [QtQuick3D] {Node::opacity} are not taken into account there.

    \note When sampling a base color map, the effective alpha value is the
    sampled alpha multiplied by the \l baseColor alpha.

    \value PrincipledMaterial.Default No test is applied, the effective alpha
    value is passed on as-is. Note that a \l baseColor or \l baseColorMap alpha
    less than \c 1.0 does not automatically imply alpha blending, the object
    with the material may still be treated as opaque, if no other relevant
    properties (such as, an opacity less than 1, the presence of an opacity
    map, or a non-default \l blendMode value) trigger treating the object as
    semi-transparent. To ensure alpha blending happens regardless of any other
    object or material property, set \c Blend instead.

    \value PrincipledMaterial.Blend No cutoff test is applied, but guarantees
    that alpha blending happens. The object with this material will therefore
    never be treated as opaque by the renderer.

    \value PrincipledMaterial.Opaque No cutoff test is applied and the rendered
    object is assumed to be fully opaque, meaning the alpha values in the
    vertex color, base color, and base color map are ignored and a value of 1.0
    is substituted instead. This mode does not guarantee alpha blending does
    not happen. If relevant properties (such as, an opacity less than 1, an
    opacity map, or a non-default \l blendMode) say so, then the object will
    still be treated as semi-transparent by the rendering pipeline, just like
    with the \c Default alphaMode.

    \value PrincipledMaterial.Mask A test based on \l alphaCutoff is applied.
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
    \qmlproperty real PrincipledMaterial::alphaCutoff

    The alphaCutoff property can be used to specify the cutoff value when using
    the \l{alphaMode}{Mask alphaMode}. Fragments where the alpha value falls
    below the threshold will be rendered fully transparent (\c{0.0} for all
    color channels). When the alpha value is equal or greater than the cutoff
    value, the color will not be affected in any way.

    The default value is 0.5.

    \sa alphaMode
*/

/*!
    \qmlproperty real PrincipledMaterial::pointSize

    This property determines the size of the points rendered, when the geometry
    is using a primitive type of points. The default value is 1.0. This
    property is not relevant when rendering other types of geometry, such as,
    triangle meshes.

    \warning Point sizes other than 1 may not be supported at run time,
    depending on the underyling graphics API. For example, setting a size other
    than 1 has no effect with Direct 3D.
*/

/*!
    \qmlproperty real PrincipledMaterial::lineWidth

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
    \qmlproperty Texture PrincipledMaterial::heightMap

    This property defines a texture used to determine the height the texture
    will be displaced when rendered through the use of Parallax Mapping. Values
    are expected to be linear from 0.0 to 1.0, where 0.0 means no displacement
    and 1.0 means means maximum displacement.

*/

/*!
    \qmlproperty enumeration PrincipledMaterial::heightChannel

    This property defines the texture channel used to read the height value
    from heightMap. The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real PrincipledMaterial::heightAmount

    This property contains the factor used to modify the values from the
    \l heightMap texture. The value should be between 0.0 to 1.0. The default
    value is 0.0 which means that height displacement will be disabled, even
    if a height map set.
*/

/*!
    \qmlproperty int PrincipledMaterial::minHeightMapSamples

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
    \qmlproperty int PrincipledMaterial::maxHeightMapSamples

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
    \qmlproperty real PrincipledMaterial::clearcoatAmount

    This property defines the intensity of the clearcoat layer.

    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture PrincipledMaterial::clearcoatMap

    This property defines a texture used to determine the intensity of the
    clearcoat layer.  The value of\l clearcoatAmount will be multiplied by
    the value read from this texture.

*/

/*!
    \qmlproperty enumeration PrincipledMaterial::clearcoatChannel

    This property defines the texture channel used to read the clearcoat amount
    value from \l clearcoatMap. The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real PrincipledMaterial::clearcoatRoughnessAmount

    This property defines the roughness of the clearcoat layer.
    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture PrincipledMaterial::clearcoatRoughnessMap

    This property defines a texture used to determine the roughness of the
    clearcoat layer.  The value of\l clearcoatRoughnessAmount will be
    multiplied by the value read from this texture.

*/

/*!
    \qmlproperty enumeration PrincipledMaterial::clearcoatRoughnessChannel

    This property defines the texture channel used to read the clearcoat
    roughness amount from \l clearcoatRoughnessMap.
    The default value is \c Material.G.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty Texture PrincipledMaterial::clearcoatNormalMap

    This property defines a texture used to determine the normal mapping
    applied to the clearcoat layer.

*/


/*!
    \qmlproperty real PrincipledMaterial::clearcoatNormalStrength

    This property controls the amount of simulated displacement for the clearcoatNormalMap.
*/

/*!
    \qmlproperty real PrincipledMaterial::transmissionFactor

    This property defines the percentage of light that is transmitted through
    the material's surface.
    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture PrincipledMaterial::transmissionMap

    This property defines a texture used to determine percentage of light that
    is transmitted through the surface..  The value of
    \l transmissionFactor will be multiplied by the value read from this
    texture.

*/

/*!
    \qmlproperty enumeration PrincipledMaterial::transmissionChannel

    This property defines the texture channel used to read the transmission
    percentage from \l transmissionMap.
    The default value is \c Material.R.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real PrincipledMaterial::thicknessFactor

    This property defines the thickness of the volume beneath the surface.
    Unlike many other properties of PrincipledMaterial, the value in defined
    in thicknessFactor is a value from 0.0 to +infinity for thickness in the
    models coordinate space.  A value of 0.0 means that the material is
    thin-walled.
    The default value is \c 0.0
*/

/*!
    \qmlproperty Texture PrincipledMaterial::thicknessMap

    This property defines a texture used to define the thickness of a
    material volume.  The value of \l thicknessFactor will be multiplied by the
    value read from this texture.

*/

/*!
    \qmlproperty enumeration PrincipledMaterial::thicknessChannel

    This property defines the texture channel used to read the thickness
    amount from \l transmissionMap.
    The default value is \c Material.G.

    \value Material.R Read value from texture R channel.
    \value Material.G Read value from texture G channel.
    \value Material.B Read value from texture B channel.
    \value Material.A Read value from texture A channel.

*/

/*!
    \qmlproperty real PrincipledMaterial::attenuationDistance

    This property defines the Density of the medium given as the average
    distance that light travels in the medium before interacting with a
    particle. The value is given in world space.
    The default value is \c +infinity.
*/

/*!
    \qmlproperty color PrincipledMaterial::attenuationColor

    This property defines the color that white lights turns into due to
    absorption when reaching the attenuation distance.
    The default value is \c Qt.White

*/

/*!
    \qmlproperty real PrincipledMaterial::indexOfRefraction

    This property defines the index of refraction of the material. The default
    value of \c 1.5 will be the ideal value for materials like plastics or glass
    but other materials like water, asphalt, sapphire, or diamond would require
    and adjusted value to look more realistic. For realistic materials the
    indexOfRefraction should usually be between \c 1.0 and \c 3.0

    Some examples of common materials' index of refractions are:

    \table
    \header
        \li Material
        \li Index of Refraction
    \row
        \li Air
        \li 1.0
    \row
        \li Water
        \li 1.33
    \row
        \li Glass
        \li 1.55
    \row
        \li Sapphire
        \li 1.76
    \row
        \li Diamond
        \li 2.42
    \endtable

    \note No known material in the world have ior much greater than \c 3.0.
*/

/*!
    \qmlproperty real PrincipledMaterial::fresnelScaleBiasEnabled

    By Setting the value to true the material will take Fresnel Scale and Fresnel Bias into account.
    The default value is \c false.
*/

/*!
    \qmlproperty real PrincipledMaterial::fresnelScale

    This property scale head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    In order to affect changes to the material you have to enable fresnelScaleBiasEnabled.
    The default value is \c 1.0.
*/

/*!
    \qmlproperty real PrincipledMaterial::fresnelBias

    This property push forward head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    In order to affect changes to the material you have to enable fresnelScaleBiasEnabled.
    The default value is \c 0.0.
*/

/*!
    \qmlproperty real PrincipledMaterial::fresnelPower

    This property decreases head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    The default value is \c 5.0.
*/

/*!
    \qmlproperty real PrincipledMaterial::clearcoatFresnelScaleBiasEnabled

    By Setting the value to true the material will take Clearcoat Fresnel Scale and Clearcoat Fresnel Bias into account.
    The default value is \c false.
*/

/*!
    \qmlproperty real PrincipledMaterial::clearcoatFresnelScale

    This property scale head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    In order to affect changes to the material you have to enable clearcoatFresnelScaleBiasEnabled.
    The default value is \c 1.0.
*/

/*!
    \qmlproperty real PrincipledMaterial::clearcoatFresnelBias

    This property push forward head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    In order to affect changes to the material you have to enable clearcoatFresnelScaleBiasEnabled.
    The default value is \c 0.0.
*/

/*!
    \qmlproperty real PrincipledMaterial::clearcoatFresnelPower

    This property decreases head-on reflections (looking directly at the
    surface) while maintaining reflections seen at grazing angles.
    The default value is \c 5.0.
*/

/*!
    \qmlproperty bool PrincipledMaterial::vertexColorsEnabled
    \since 6.5

    When this property is enabled, the material will use vertex colors from the
    mesh. These will be multiplied by any other colors specified for the
    material. The default value is true.
*/

/*!
    \qmlproperty bool PrincipledMaterial::vertexColorsMaskEnabled
    \since 6.8

    When this property is enabled, the material will use vertex colors from the
    mesh as mask of various properties e.g RoughnessAmount, SpecularAmount, ... .
    The default value is false.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::vertexColorRedMask
    \since 6.8

    This property defines the vertex color red channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c PrincipledMaterial.NoMask.

    \value PrincipledMaterial.NoMask.
    \value PrincipledMaterial.ClearcoatAmountMask.
    \value PrincipledMaterial.ClearcoatRoughnessAmountMask.
    \value PrincipledMaterial.ClearcoatNormalStrengthMask.
    \value PrincipledMaterial.HeightAmountMask.
    \value PrincipledMaterial.MetalnessMask.
    \value PrincipledMaterial.RoughnessMask.
    \value PrincipledMaterial.NormalStrengthMask.
    \value PrincipledMaterial.OcclusionAmountMask.
    \value PrincipledMaterial.SpecularAmountMask.
    \value PrincipledMaterial.ThicknessFactorMask.
    \value PrincipledMaterial.TransmissionFactorMask.
 */

/*!
    \qmlproperty enumeration PrincipledMaterial::vertexColorGreenMask
    \since 6.8

    This property defines the vertex color green channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c PrincipledMaterial.NoMask.

    \value PrincipledMaterial.NoMask.
    \value PrincipledMaterial.ClearcoatAmountMask.
    \value PrincipledMaterial.ClearcoatRoughnessAmountMask.
    \value PrincipledMaterial.ClearcoatNormalStrengthMask.
    \value PrincipledMaterial.HeightAmountMask.
    \value PrincipledMaterial.MetalnessMask.
    \value PrincipledMaterial.RoughnessMask.
    \value PrincipledMaterial.NormalStrengthMask.
    \value PrincipledMaterial.OcclusionAmountMask.
    \value PrincipledMaterial.SpecularAmountMask.
    \value PrincipledMaterial.ThicknessFactorMask.
    \value PrincipledMaterial.TransmissionFactorMask.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::vertexColorBlueMask
    \since 6.8

    This property defines the vertex color blue channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c PrincipledMaterial.NoMask.

    \value PrincipledMaterial.NoMask.
    \value PrincipledMaterial.ClearcoatAmountMask.
    \value PrincipledMaterial.ClearcoatRoughnessAmountMask.
    \value PrincipledMaterial.ClearcoatNormalStrengthMask.
    \value PrincipledMaterial.HeightAmountMask.
    \value PrincipledMaterial.MetalnessMask.
    \value PrincipledMaterial.RoughnessMask.
    \value PrincipledMaterial.NormalStrengthMask.
    \value PrincipledMaterial.OcclusionAmountMask.
    \value PrincipledMaterial.SpecularAmountMask.
    \value PrincipledMaterial.ThicknessFactorMask.
    \value PrincipledMaterial.TransmissionFactorMask.
*/

/*!
    \qmlproperty enumeration PrincipledMaterial::vertexColorAlphaMask
    \since 6.8

    This property defines the vertex color alpha channel used as the specifies mask.
    The value is a bit-wise combination of flags.
    The default value is \c PrincipledMaterial.NoMask.

    \value PrincipledMaterial.NoMask.
    \value PrincipledMaterial.ClearcoatAmountMask.
    \value PrincipledMaterial.ClearcoatRoughnessAmountMask.
    \value PrincipledMaterial.ClearcoatNormalStrengthMask.
    \value PrincipledMaterial.HeightAmountMask.
    \value PrincipledMaterial.MetalnessMask.
    \value PrincipledMaterial.RoughnessMask.
    \value PrincipledMaterial.NormalStrengthMask.
    \value PrincipledMaterial.OcclusionAmountMask.
    \value PrincipledMaterial.SpecularAmountMask.
    \value PrincipledMaterial.ThicknessFactorMask.
    \value PrincipledMaterial.TransmissionFactorMask.
*/

QQuick3DPrincipledMaterial::QQuick3DPrincipledMaterial(QQuick3DObject *parent)
    : QQuick3DMaterial(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::PrincipledMaterial)), parent)
{}

QQuick3DPrincipledMaterial::~QQuick3DPrincipledMaterial()
{
}

QQuick3DPrincipledMaterial::Lighting QQuick3DPrincipledMaterial::lighting() const
{
    return m_lighting;
}

QQuick3DPrincipledMaterial::BlendMode QQuick3DPrincipledMaterial::blendMode() const
{
    return m_blendMode;
}

QColor QQuick3DPrincipledMaterial::baseColor() const
{
    return m_baseColor;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::baseColorMap() const
{
    return m_baseColorMap;
}

bool QQuick3DPrincipledMaterial::baseColorSingleChannelEnabled() const
{
    return m_baseColorSingleChannelEnabled;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::baseColorChannel() const
{
    return m_baseColorChannel;
}

bool QQuick3DPrincipledMaterial::specularSingleChannelEnabled() const
{
    return m_specularSingleChannelEnabled;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::specularChannel() const
{
    return m_specularChannel;
}

bool QQuick3DPrincipledMaterial::emissiveSingleChannelEnabled() const
{
    return m_emissiveSingleChannelEnabled;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::emissiveChannel() const
{
    return m_emissiveChannel;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::emissiveMap() const
{
    return m_emissiveMap;
}

QVector3D QQuick3DPrincipledMaterial::emissiveFactor() const
{
    return m_emissiveFactor;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::specularReflectionMap() const
{
    return m_specularReflectionMap;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::specularMap() const
{
    return m_specularMap;
}

float QQuick3DPrincipledMaterial::specularTint() const
{
    return m_specularTint;
}

float QQuick3DPrincipledMaterial::specularAmount() const
{
    return m_specularAmount;
}

float QQuick3DPrincipledMaterial::roughness() const
{
    return m_roughness;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::roughnessMap() const
{
    return m_roughnessMap;
}

bool QQuick3DPrincipledMaterial::invertOpacityMapValue() const
{
    return m_invertOpacityMapValue;
}

float QQuick3DPrincipledMaterial::opacity() const
{
    return m_opacity;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::opacityMap() const
{
    return m_opacityMap;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::normalMap() const
{
    return m_normalMap;
}

float QQuick3DPrincipledMaterial::metalness() const
{
    return m_metalnessAmount;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::metalnessMap() const
{
    return m_metalnessMap;
}

float QQuick3DPrincipledMaterial::normalStrength() const
{
    return m_normalStrength;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::occlusionMap() const
{
    return m_occlusionMap;
}

float QQuick3DPrincipledMaterial::occlusionAmount() const
{
    return m_occlusionAmount;
}

QQuick3DPrincipledMaterial::AlphaMode QQuick3DPrincipledMaterial::alphaMode() const
{
    return m_alphaMode;
}

float QQuick3DPrincipledMaterial::alphaCutoff() const
{
    return m_alphaCutoff;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::roughnessChannel() const
{
    return m_roughnessChannel;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::opacityChannel() const
{
    return m_opacityChannel;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::metalnessChannel() const
{
    return m_metalnessChannel;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::occlusionChannel() const
{
    return m_occlusionChannel;
}

float QQuick3DPrincipledMaterial::pointSize() const
{
    return m_pointSize;
}

float QQuick3DPrincipledMaterial::lineWidth() const
{
    return m_lineWidth;
}

QQuick3DTexture *QQuick3DPrincipledMaterial::heightMap() const
{
    return m_heightMap;
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::heightChannel() const
{
    return m_heightChannel;
}

float QQuick3DPrincipledMaterial::heightAmount() const
{
    return m_heightAmount;
}

int QQuick3DPrincipledMaterial::minHeightMapSamples() const
{
    return m_minHeightMapSamples;
}

int QQuick3DPrincipledMaterial::maxHeightMapSamples() const
{
    return m_maxHeightMapSamples;
}

void QQuick3DPrincipledMaterial::markAllDirty()
{
    m_dirtyAttributes = 0xffffffff;
    QQuick3DMaterial::markAllDirty();
}

void QQuick3DPrincipledMaterial::setLighting(QQuick3DPrincipledMaterial::Lighting lighting)
{
    if (m_lighting == lighting)
        return;

    m_lighting = lighting;
    emit lightingChanged(m_lighting);
    markDirty(LightingModeDirty);
}

void QQuick3DPrincipledMaterial::setBlendMode(QQuick3DPrincipledMaterial::BlendMode blendMode)
{
    if (m_blendMode == blendMode)
        return;

    m_blendMode = blendMode;
    emit blendModeChanged(m_blendMode);
    markDirty(BlendModeDirty);
}

void QQuick3DPrincipledMaterial::setBaseColor(QColor diffuseColor)
{
    if (m_baseColor == diffuseColor)
        return;

    m_baseColor = diffuseColor;
    emit baseColorChanged(m_baseColor);
    markDirty(BaseColorDirty);
}

void QQuick3DPrincipledMaterial::setBaseColorMap(QQuick3DTexture *baseColorMap)
{
    if (m_baseColorMap == baseColorMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setBaseColorMap, baseColorMap, m_baseColorMap);

    m_baseColorMap = baseColorMap;
    emit baseColorMapChanged(m_baseColorMap);
    markDirty(BaseColorDirty);
}

void QQuick3DPrincipledMaterial::setBaseColorSingleChannelEnabled(bool baseColorSingleChannelEnabled)
{
    if (m_baseColorSingleChannelEnabled == baseColorSingleChannelEnabled)
        return;

    m_baseColorSingleChannelEnabled = baseColorSingleChannelEnabled;
    emit baseColorSingleChannelEnabledChanged(baseColorSingleChannelEnabled);
    markDirty(BaseColorDirty);
}

void QQuick3DPrincipledMaterial::setBaseColorChannel(TextureChannelMapping channel)
{
    if (m_baseColorChannel == channel)
        return;

    m_baseColorChannel = channel;
    emit baseColorChannelChanged(channel);
    markDirty(BaseColorDirty);
}

void QQuick3DPrincipledMaterial::setSpecularSingleChannelEnabled(bool specularSingleChannelEnabled)
{
    if (m_specularSingleChannelEnabled == specularSingleChannelEnabled)
        return;

    m_specularSingleChannelEnabled = specularSingleChannelEnabled;
    emit specularSingleChannelEnabledChanged(specularSingleChannelEnabled);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setSpecularChannel(TextureChannelMapping channel)
{
    if (m_specularChannel == channel)
        return;

    m_specularChannel = channel;
    emit specularChannelChanged(channel);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setEmissiveSingleChannelEnabled(bool emissiveSingleChannelEnabled)
{
    if (m_emissiveSingleChannelEnabled == emissiveSingleChannelEnabled)
        return;

    m_emissiveSingleChannelEnabled = emissiveSingleChannelEnabled;
    emit emissiveSingleChannelEnabledChanged(emissiveSingleChannelEnabled);
    markDirty(EmissiveDirty);
}

void QQuick3DPrincipledMaterial::setEmissiveChannel(TextureChannelMapping channel)
{
    if (m_emissiveChannel == channel)
        return;

    m_emissiveChannel = channel;
    emit emissiveChannelChanged(channel);
    markDirty(EmissiveDirty);
}

void QQuick3DPrincipledMaterial::setEmissiveMap(QQuick3DTexture *emissiveMap)
{
    if (m_emissiveMap == emissiveMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setEmissiveMap, emissiveMap, m_emissiveMap);

    m_emissiveMap = emissiveMap;
    emit emissiveMapChanged(m_emissiveMap);
    markDirty(EmissiveDirty);
}

void QQuick3DPrincipledMaterial::setEmissiveFactor(QVector3D emissiveFactor)
{
    if (m_emissiveFactor == emissiveFactor)
        return;

    m_emissiveFactor = emissiveFactor;
    emit emissiveFactorChanged(m_emissiveFactor);
    markDirty(EmissiveDirty);
}

void QQuick3DPrincipledMaterial::setSpecularReflectionMap(QQuick3DTexture *specularReflectionMap)
{
    if (m_specularReflectionMap == specularReflectionMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setSpecularReflectionMap, specularReflectionMap, m_specularReflectionMap);

    m_specularReflectionMap = specularReflectionMap;
    emit specularReflectionMapChanged(m_specularReflectionMap);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setSpecularMap(QQuick3DTexture *specularMap)
{
    if (m_specularMap == specularMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setSpecularMap, specularMap, m_specularMap);

    m_specularMap = specularMap;
    emit specularMapChanged(m_specularMap);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setSpecularTint(float specularTint)
{
    specularTint = ensureNormalized(specularTint);
    if (qFuzzyCompare(m_specularTint, specularTint))
        return;

    m_specularTint = specularTint;
    emit specularTintChanged(m_specularTint);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setSpecularAmount(float specularAmount)
{
    specularAmount = ensureNormalized(specularAmount);
    if (qFuzzyCompare(m_specularAmount, specularAmount))
        return;

    m_specularAmount = specularAmount;
    emit specularAmountChanged(m_specularAmount);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setRoughness(float roughness)
{
    roughness = ensureNormalized(roughness);
    if (qFuzzyCompare(m_roughness, roughness))
        return;

    m_roughness = roughness;
    emit roughnessChanged(m_roughness);
    markDirty(RoughnessDirty);
}

void QQuick3DPrincipledMaterial::setRoughnessMap(QQuick3DTexture *roughnessMap)
{
    if (m_roughnessMap == roughnessMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setRoughnessMap, roughnessMap, m_roughnessMap);

    m_roughnessMap = roughnessMap;
    emit roughnessMapChanged(m_roughnessMap);
    markDirty(RoughnessDirty);
}

void QQuick3DPrincipledMaterial::setInvertOpacityMapValue(bool invertOpacityMapValue)
{
    if (invertOpacityMapValue == m_invertOpacityMapValue)
        return;

    m_invertOpacityMapValue = invertOpacityMapValue;
    emit invertOpacityMapValueChanged(m_invertOpacityMapValue);
    markDirty(OpacityDirty);
}

void QQuick3DPrincipledMaterial::setOpacity(float opacity)
{
    opacity = ensureNormalized(opacity);
    if (qFuzzyCompare(m_opacity, opacity))
        return;

    m_opacity = opacity;
    emit opacityChanged(m_opacity);
    markDirty(OpacityDirty);
}

void QQuick3DPrincipledMaterial::setOpacityMap(QQuick3DTexture *opacityMap)
{
    if (m_opacityMap == opacityMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &::QQuick3DPrincipledMaterial::setOpacityMap, opacityMap, m_opacityMap);

    m_opacityMap = opacityMap;
    emit opacityMapChanged(m_opacityMap);
    markDirty(OpacityDirty);
}

void QQuick3DPrincipledMaterial::setNormalMap(QQuick3DTexture *normalMap)
{
    if (m_normalMap == normalMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setNormalMap, normalMap, m_normalMap);

    m_normalMap = normalMap;
    emit normalMapChanged(m_normalMap);
    markDirty(NormalDirty);
}

void QQuick3DPrincipledMaterial::setMetalness(float metalnessAmount)
{
    metalnessAmount = ensureNormalized(metalnessAmount);
    if (qFuzzyCompare(m_metalnessAmount, metalnessAmount))
        return;

    m_metalnessAmount = metalnessAmount;
    emit metalnessChanged(m_metalnessAmount);
    markDirty(MetalnessDirty);
}

void QQuick3DPrincipledMaterial::setMetalnessMap(QQuick3DTexture *metallicMap)
{
    if (m_metalnessMap == metallicMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setMetalnessMap, metallicMap, m_metalnessMap);

    m_metalnessMap = metallicMap;
    emit metalnessMapChanged(m_metalnessMap);
    markDirty(MetalnessDirty);
}

void QQuick3DPrincipledMaterial::setNormalStrength(float factor)
{
    factor = ensureNormalized(factor);
    if (qFuzzyCompare(m_normalStrength, factor))
        return;

    m_normalStrength = factor;
    emit normalStrengthChanged(m_normalStrength);
    markDirty(NormalDirty);
}

void QQuick3DPrincipledMaterial::setOcclusionMap(QQuick3DTexture *occlusionMap)
{
    if (m_occlusionMap == occlusionMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setOcclusionMap, occlusionMap, m_occlusionMap);

    m_occlusionMap = occlusionMap;
    emit occlusionMapChanged(m_occlusionMap);
    markDirty(OcclusionDirty);
}

void QQuick3DPrincipledMaterial::setOcclusionAmount(float occlusionAmount)
{
    if (qFuzzyCompare(m_occlusionAmount, occlusionAmount))
        return;

    m_occlusionAmount = occlusionAmount;
    emit occlusionAmountChanged(m_occlusionAmount);
    markDirty(OcclusionDirty);
}

void QQuick3DPrincipledMaterial::setAlphaMode(QQuick3DPrincipledMaterial::AlphaMode alphaMode)
{
    if (m_alphaMode == alphaMode)
        return;

    m_alphaMode = alphaMode;
    emit alphaModeChanged(m_alphaMode);
    markDirty(AlphaModeDirty);
}

void QQuick3DPrincipledMaterial::setAlphaCutoff(float alphaCutoff)
{
    if (qFuzzyCompare(m_alphaCutoff, alphaCutoff))
        return;

    m_alphaCutoff = alphaCutoff;
    emit alphaCutoffChanged(m_alphaCutoff);
    markDirty(AlphaModeDirty);
}

void QQuick3DPrincipledMaterial::setMetalnessChannel(TextureChannelMapping channel)
{
    if (m_metalnessChannel == channel)
        return;

    m_metalnessChannel = channel;
    emit metalnessChannelChanged(channel);
    markDirty(MetalnessDirty);
}

void QQuick3DPrincipledMaterial::setRoughnessChannel(TextureChannelMapping channel)
{
    if (m_roughnessChannel == channel)
        return;

    m_roughnessChannel = channel;
    emit roughnessChannelChanged(channel);
    markDirty(RoughnessDirty);
}

void QQuick3DPrincipledMaterial::setOpacityChannel(TextureChannelMapping channel)
{
    if (m_opacityChannel == channel)
        return;

    m_opacityChannel = channel;
    emit opacityChannelChanged(channel);
    markDirty(OpacityDirty);
}

void QQuick3DPrincipledMaterial::setOcclusionChannel(TextureChannelMapping channel)
{
    if (m_occlusionChannel == channel)
        return;

    m_occlusionChannel = channel;
    emit occlusionChannelChanged(channel);
    markDirty(OcclusionDirty);
}

void QQuick3DPrincipledMaterial::setPointSize(float size)
{
    if (qFuzzyCompare(m_pointSize, size))
        return;
    m_pointSize = size;
    emit pointSizeChanged();
    markDirty(PointSizeDirty);
}

void QQuick3DPrincipledMaterial::setLineWidth(float width)
{
    if (qFuzzyCompare(m_lineWidth, width))
        return;
    m_lineWidth = width;
    emit lineWidthChanged();
    markDirty(LineWidthDirty);
}

void QQuick3DPrincipledMaterial::setHeightMap(QQuick3DTexture *heightMap)
{
    if (m_heightMap == heightMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setHeightMap, heightMap, m_heightMap);

    m_heightMap = heightMap;
    emit heightMapChanged(m_heightMap);
    markDirty(HeightDirty);
}

void QQuick3DPrincipledMaterial::setHeightChannel(QQuick3DMaterial::TextureChannelMapping channel)
{
    if (m_heightChannel == channel)
        return;

    m_heightChannel = channel;
    emit heightChannelChanged(m_heightChannel);
    markDirty(HeightDirty);
}

void QQuick3DPrincipledMaterial::setHeightAmount(float heightAmount)
{
    if (m_heightAmount == heightAmount)
        return;

    m_heightAmount = heightAmount;
    emit heightAmountChanged(m_heightAmount);
    markDirty(HeightDirty);
}

void QQuick3DPrincipledMaterial::setMinHeightMapSamples(int samples)
{
    if (m_minHeightMapSamples == samples)
        return;

    m_minHeightMapSamples = samples;
    emit minHeightMapSamplesChanged(samples);
    markDirty(HeightDirty);
}

void QQuick3DPrincipledMaterial::setMaxHeightMapSamples(int samples)
{
    if (m_maxHeightMapSamples == samples)
        return;

    m_maxHeightMapSamples = samples;
    emit maxHeightMapSamplesChanged(samples);
    markDirty(HeightDirty);
}

QSSGRenderGraphObject *QQuick3DPrincipledMaterial::updateSpatialNode(QSSGRenderGraphObject *node)
{
    static const auto channelMapping = [](TextureChannelMapping mapping) {
        return QSSGRenderDefaultMaterial::TextureChannelMapping(mapping);
    };

    if (!node) {
        markAllDirty();
        node = new QSSGRenderDefaultMaterial(QSSGRenderGraphObject::Type::PrincipledMaterial);
    }

    // Set common material properties
    QQuick3DMaterial::updateSpatialNode(node);

    QSSGRenderDefaultMaterial *material = static_cast<QSSGRenderDefaultMaterial *>(node);

    material->specularModel = QSSGRenderDefaultMaterial::MaterialSpecularModel::SchlickGGX;

    if (m_dirtyAttributes & LightingModeDirty)
        material->lighting = QSSGRenderDefaultMaterial::MaterialLighting(m_lighting);

    if (m_dirtyAttributes & BlendModeDirty)
        material->blendMode = QSSGRenderDefaultMaterial::MaterialBlendMode(m_blendMode);

    if (m_dirtyAttributes & BaseColorDirty) {
        if (!m_baseColorMap)
            material->colorMap = nullptr;
        else
            material->colorMap = m_baseColorMap->getRenderImage();

        material->color = QSSGUtils::color::sRGBToLinear(m_baseColor);
        material->baseColorSingleChannelEnabled = m_baseColorSingleChannelEnabled;
        material->baseColorChannel = channelMapping(m_baseColorChannel);
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

    if (m_dirtyAttributes & RoughnessDirty) {
        if (!m_roughnessMap)
            material->roughnessMap = nullptr;
        else
            material->roughnessMap = m_roughnessMap->getRenderImage();

        material->specularRoughness = m_roughness;
        material->roughnessChannel = channelMapping(m_roughnessChannel);
    }

    if (m_dirtyAttributes & MetalnessDirty) {
        if (!m_metalnessMap)
            material->metalnessMap = nullptr;
        else
            material->metalnessMap = m_metalnessMap->getRenderImage();

        material->metalnessAmount = m_metalnessAmount;
        material->metalnessChannel = channelMapping(m_metalnessChannel);

    }

    if (m_dirtyAttributes & SpecularDirty) {
        if (!m_specularReflectionMap)
            material->specularReflection = nullptr;
        else
            material->specularReflection = m_specularReflectionMap->getRenderImage();

        if (!m_specularMap) {
            material->specularMap = nullptr;
        } else {
            material->specularMap = m_specularMap->getRenderImage();
        }

        material->specularAmount = m_specularAmount;
        material->specularTint = QVector3D(m_specularTint, m_specularTint, m_specularTint);
        material->ior = m_indexOfRefraction;
        material->fresnelScaleBiasEnabled = m_fresnelScaleBiasEnabled;
        material->fresnelScale = m_fresnelScale;
        material->fresnelBias = m_fresnelBias;
        material->fresnelPower = m_fresnelPower;
        material->specularAmountSingleChannelEnabled = m_specularSingleChannelEnabled;
        material->specularAmountChannel = channelMapping(m_specularChannel);
    }

    if (m_dirtyAttributes & OpacityDirty) {
        material->opacity = m_opacity;
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

void QQuick3DPrincipledMaterial::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

void QQuick3DPrincipledMaterial::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    // Check all the resource value's scene manager, and update as necessary.
    if (sceneManager) {
        QQuick3DObjectPrivate::refSceneManager(m_baseColorMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_emissiveMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_specularReflectionMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_specularMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_roughnessMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_opacityMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_normalMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_metalnessMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_occlusionMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_heightMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_clearcoatMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_clearcoatRoughnessMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_clearcoatNormalMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_transmissionMap, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_thicknessMap, *sceneManager);
    } else {
        QQuick3DObjectPrivate::derefSceneManager(m_baseColorMap);
        QQuick3DObjectPrivate::derefSceneManager(m_emissiveMap);
        QQuick3DObjectPrivate::derefSceneManager(m_specularReflectionMap);
        QQuick3DObjectPrivate::derefSceneManager(m_specularMap);
        QQuick3DObjectPrivate::derefSceneManager(m_roughnessMap);
        QQuick3DObjectPrivate::derefSceneManager(m_opacityMap);
        QQuick3DObjectPrivate::derefSceneManager(m_normalMap);
        QQuick3DObjectPrivate::derefSceneManager(m_metalnessMap);
        QQuick3DObjectPrivate::derefSceneManager(m_occlusionMap);
        QQuick3DObjectPrivate::derefSceneManager(m_heightMap);
        QQuick3DObjectPrivate::derefSceneManager(m_clearcoatMap);
        QQuick3DObjectPrivate::derefSceneManager(m_clearcoatRoughnessMap);
        QQuick3DObjectPrivate::derefSceneManager(m_clearcoatNormalMap);
        QQuick3DObjectPrivate::derefSceneManager(m_transmissionMap);
        QQuick3DObjectPrivate::derefSceneManager(m_thicknessMap);
    }
}

void QQuick3DPrincipledMaterial::markDirty(QQuick3DPrincipledMaterial::DirtyType type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

float QQuick3DPrincipledMaterial::clearcoatAmount() const
{
    return m_clearcoatAmount;
}

void QQuick3DPrincipledMaterial::setClearcoatAmount(float newClearcoatAmount)
{
    if (qFuzzyCompare(m_clearcoatAmount, newClearcoatAmount))
        return;
    m_clearcoatAmount = newClearcoatAmount;
    emit clearcoatAmountChanged(m_clearcoatAmount);
    markDirty(ClearcoatDirty);
}

QQuick3DTexture *QQuick3DPrincipledMaterial::clearcoatMap() const
{
    return m_clearcoatMap;
}

void QQuick3DPrincipledMaterial::setClearcoatMap(QQuick3DTexture *newClearcoatMap)
{
    if (m_clearcoatMap == newClearcoatMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setClearcoatMap, newClearcoatMap, m_clearcoatMap);

    m_clearcoatMap = newClearcoatMap;
    emit clearcoatMapChanged(m_clearcoatMap);
    markDirty(ClearcoatDirty);
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::clearcoatChannel() const
{
    return m_clearcoatChannel;
}

void QQuick3DPrincipledMaterial::setClearcoatChannel(QQuick3DMaterial::TextureChannelMapping newClearcoatChannel)
{
    if (m_clearcoatChannel == newClearcoatChannel)
        return;
    m_clearcoatChannel = newClearcoatChannel;
    emit clearcoatChannelChanged(m_clearcoatChannel);
    markDirty(ClearcoatDirty);
}

float QQuick3DPrincipledMaterial::clearcoatRoughnessAmount() const
{
    return m_clearcoatRoughnessAmount;
}

void QQuick3DPrincipledMaterial::setClearcoatRoughnessAmount(float newClearcoatRoughnessAmount)
{
    if (qFuzzyCompare(m_clearcoatRoughnessAmount, newClearcoatRoughnessAmount))
        return;
    m_clearcoatRoughnessAmount = newClearcoatRoughnessAmount;
    emit clearcoatRoughnessAmountChanged(m_clearcoatRoughnessAmount);
    markDirty(ClearcoatDirty);
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::clearcoatRoughnessChannel() const
{
    return m_clearcoatRoughnessChannel;
}

void QQuick3DPrincipledMaterial::setClearcoatRoughnessChannel(QQuick3DMaterial::TextureChannelMapping newClearcoatRoughnessChannel)
{
    if (m_clearcoatRoughnessChannel == newClearcoatRoughnessChannel)
        return;
    m_clearcoatRoughnessChannel = newClearcoatRoughnessChannel;
    emit clearcoatRoughnessChannelChanged(m_clearcoatRoughnessChannel);
    markDirty(ClearcoatDirty);
}

QQuick3DTexture *QQuick3DPrincipledMaterial::clearcoatRoughnessMap() const
{
    return m_clearcoatRoughnessMap;
}

void QQuick3DPrincipledMaterial::setClearcoatRoughnessMap(QQuick3DTexture *newClearcoatRoughnessMap)
{
    if (m_clearcoatRoughnessMap == newClearcoatRoughnessMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setClearcoatRoughnessMap, newClearcoatRoughnessMap, m_clearcoatRoughnessMap);

    m_clearcoatRoughnessMap = newClearcoatRoughnessMap;
    emit clearcoatRoughnessMapChanged(m_clearcoatRoughnessMap);
    markDirty(ClearcoatDirty);
}

QQuick3DTexture *QQuick3DPrincipledMaterial::clearcoatNormalMap() const
{
    return m_clearcoatNormalMap;
}

void QQuick3DPrincipledMaterial::setClearcoatNormalMap(QQuick3DTexture *newClearcoatNormalMap)
{
    if (m_clearcoatNormalMap == newClearcoatNormalMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setClearcoatNormalMap, newClearcoatNormalMap, m_clearcoatNormalMap);

    m_clearcoatNormalMap = newClearcoatNormalMap;
    emit clearcoatNormalMapChanged(m_clearcoatNormalMap);
    markDirty(ClearcoatDirty);
}


float QQuick3DPrincipledMaterial::clearcoatNormalStrength() const
{
    return m_clearcoatNormalStrength;
}

void QQuick3DPrincipledMaterial::setClearcoatNormalStrength(float newClearcoatNormalStrength)
{
    if (qFuzzyCompare(m_clearcoatNormalStrength, newClearcoatNormalStrength))
        return;

    m_clearcoatNormalStrength = newClearcoatNormalStrength;
    emit clearcoatNormalStrengthChanged(m_clearcoatNormalStrength);
    markDirty(ClearcoatDirty);
}

float QQuick3DPrincipledMaterial::transmissionFactor() const
{
    return m_transmissionFactor;
}

void QQuick3DPrincipledMaterial::setTransmissionFactor(float newTransmissionFactor)
{
    if (qFuzzyCompare(m_transmissionFactor, newTransmissionFactor))
        return;
    m_transmissionFactor = newTransmissionFactor;
    emit transmissionFactorChanged(m_transmissionFactor);
    markDirty(TransmissionDirty);
}

QQuick3DTexture *QQuick3DPrincipledMaterial::transmissionMap() const
{
    return m_transmissionMap;
}

void QQuick3DPrincipledMaterial::setTransmissionMap(QQuick3DTexture *newTransmissionMap)
{
    if (m_transmissionMap == newTransmissionMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setTransmissionMap, newTransmissionMap, m_transmissionMap);

    m_transmissionMap = newTransmissionMap;
    emit transmissionMapChanged(m_transmissionMap);
    markDirty(TransmissionDirty);
}

QQuick3DMaterial::TextureChannelMapping QQuick3DPrincipledMaterial::transmissionChannel() const
{
    return m_transmissionChannel;
}

float QQuick3DPrincipledMaterial::indexOfRefraction() const
{
    return m_indexOfRefraction;
}

bool QQuick3DPrincipledMaterial::fresnelScaleBiasEnabled() const
{
    return m_fresnelScaleBiasEnabled;
}

float QQuick3DPrincipledMaterial::fresnelScale() const
{
    return m_fresnelScale;
}

float QQuick3DPrincipledMaterial::fresnelBias() const
{
    return m_fresnelBias;
}

float QQuick3DPrincipledMaterial::fresnelPower() const
{
    return m_fresnelPower;
}

bool QQuick3DPrincipledMaterial::clearcoatFresnelScaleBiasEnabled() const
{
    return m_clearcoatFresnelScaleBiasEnabled;
}

float QQuick3DPrincipledMaterial::clearcoatFresnelScale() const
{
    return m_clearcoatFresnelScale;
}

float QQuick3DPrincipledMaterial::clearcoatFresnelBias() const
{
    return m_clearcoatFresnelBias;
}

float QQuick3DPrincipledMaterial::clearcoatFresnelPower() const
{
    return m_clearcoatFresnelPower;
}

bool QQuick3DPrincipledMaterial::vertexColorsEnabled() const
{
    return m_vertexColorsEnabled;
}

void QQuick3DPrincipledMaterial::setTransmissionChannel(QQuick3DMaterial::TextureChannelMapping newTransmissionChannel)
{
    if (m_transmissionChannel == newTransmissionChannel)
        return;
    m_transmissionChannel = newTransmissionChannel;
    emit transmissionChannelChanged(m_transmissionChannel);
    markDirty(TransmissionDirty);
}

float QQuick3DPrincipledMaterial::thicknessFactor() const
{
    return m_thicknessFactor;
}

void QQuick3DPrincipledMaterial::setThicknessFactor(float newThicknessFactor)
{
    if (qFuzzyCompare(m_thicknessFactor, newThicknessFactor))
        return;
    m_thicknessFactor = newThicknessFactor;
    emit thicknessFactorChanged(m_thicknessFactor);
    markDirty(VolumeDirty);
}

QQuick3DTexture *QQuick3DPrincipledMaterial::thicknessMap() const
{
    return m_thicknessMap;
}

void QQuick3DPrincipledMaterial::setThicknessMap(QQuick3DTexture *newThicknessMap)
{
    if (m_thicknessMap == newThicknessMap)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DPrincipledMaterial::setThicknessMap, newThicknessMap, m_thicknessMap);

    m_thicknessMap = newThicknessMap;
    emit thicknessMapChanged(m_thicknessMap);
    markDirty(VolumeDirty);
}

const QQuick3DMaterial::TextureChannelMapping &QQuick3DPrincipledMaterial::thicknessChannel() const
{
    return m_thicknessChannel;
}

void QQuick3DPrincipledMaterial::setThicknessChannel(const QQuick3DMaterial::TextureChannelMapping &newThicknessChannel)
{
    if (m_thicknessChannel == newThicknessChannel)
        return;
    m_thicknessChannel = newThicknessChannel;
    emit thicknessChannelChanged(m_thicknessChannel);
    markDirty(VolumeDirty);
}

float QQuick3DPrincipledMaterial::attenuationDistance() const
{
    return m_attenuationDistance;
}

void QQuick3DPrincipledMaterial::setAttenuationDistance(float newAttenuationDistance)
{
    if (qFuzzyCompare(m_attenuationDistance, newAttenuationDistance))
        return;
    m_attenuationDistance = newAttenuationDistance;
    emit attenuationDistanceChanged(m_attenuationDistance);
    markDirty(VolumeDirty);
}

const QColor &QQuick3DPrincipledMaterial::attenuationColor() const
{
    return m_attenuationColor;
}

void QQuick3DPrincipledMaterial::setAttenuationColor(const QColor &newAttenuationColor)
{
    if (m_attenuationColor == newAttenuationColor)
        return;
    m_attenuationColor = newAttenuationColor;
    emit attenuationColorChanged(m_attenuationColor);
    markDirty(VolumeDirty);
}

void QQuick3DPrincipledMaterial::setIndexOfRefraction(float indexOfRefraction)
{
    if (qFuzzyCompare(m_indexOfRefraction, indexOfRefraction))
        return;

    m_indexOfRefraction = indexOfRefraction;
    emit indexOfRefractionChanged(m_indexOfRefraction);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setFresnelScaleBiasEnabled(bool fresnelScaleBiasEnabled)
{
    if (m_fresnelScaleBiasEnabled == fresnelScaleBiasEnabled)
        return;

    m_fresnelScaleBiasEnabled = fresnelScaleBiasEnabled;
    emit fresnelScaleBiasEnabledChanged(m_fresnelScaleBiasEnabled);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setFresnelScale(float fresnelScale)
{
    if (qFuzzyCompare(m_fresnelScale, fresnelScale))
        return;

    m_fresnelScale = fresnelScale;
    emit fresnelScaleChanged(m_fresnelScale);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setFresnelBias(float fresnelBias)
{
    if (qFuzzyCompare(m_fresnelBias, fresnelBias))
        return;

    m_fresnelBias = fresnelBias;
    emit fresnelBiasChanged(m_fresnelBias);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setFresnelPower(float fresnelPower)
{
    if (qFuzzyCompare(m_fresnelPower, fresnelPower))
        return;

    m_fresnelPower = fresnelPower;
    emit fresnelPowerChanged(m_fresnelPower);
    markDirty(SpecularDirty);
}

void QQuick3DPrincipledMaterial::setClearcoatFresnelScaleBiasEnabled(bool clearcoatFresnelScaleBiasEnabled)
{
    if (m_clearcoatFresnelScaleBiasEnabled == clearcoatFresnelScaleBiasEnabled)
        return;

    m_clearcoatFresnelScaleBiasEnabled = clearcoatFresnelScaleBiasEnabled;
    emit clearcoatFresnelScaleBiasEnabledChanged(m_clearcoatFresnelScaleBiasEnabled);
    markDirty(ClearcoatDirty);
}

void QQuick3DPrincipledMaterial::setClearcoatFresnelScale(float clearcoatFresnelScale)
{
    if (qFuzzyCompare(m_clearcoatFresnelScale, clearcoatFresnelScale))
        return;

    m_clearcoatFresnelScale = clearcoatFresnelScale;
    emit clearcoatFresnelScaleChanged(m_clearcoatFresnelScale);
    markDirty(ClearcoatDirty);
}

void QQuick3DPrincipledMaterial::setClearcoatFresnelBias(float clearcoatFresnelBias)
{
    if (qFuzzyCompare(m_clearcoatFresnelBias, clearcoatFresnelBias))
        return;

    m_clearcoatFresnelBias = clearcoatFresnelBias;
    emit clearcoatFresnelBiasChanged(m_clearcoatFresnelBias);
    markDirty(ClearcoatDirty);
}

void QQuick3DPrincipledMaterial::setClearcoatFresnelPower(float clearcoatFresnelPower)
{
    if (qFuzzyCompare(m_clearcoatFresnelPower, clearcoatFresnelPower))
        return;

    m_clearcoatFresnelPower = clearcoatFresnelPower;
    emit clearcoatFresnelPowerChanged(m_clearcoatFresnelPower);
    markDirty(ClearcoatDirty);
}

void QQuick3DPrincipledMaterial::setVertexColorsEnabled(bool vertexColors)
{
    if (m_vertexColorsEnabled == vertexColors)
        return;

    m_vertexColorsEnabled = vertexColors;
    emit vertexColorsEnabledChanged(m_vertexColorsEnabled);
    markDirty(VertexColorsDirty);
}

bool QQuick3DPrincipledMaterial::vertexColorsMaskEnabled() const
{
    return m_vertexColorsMaskEnabled;
}

void QQuick3DPrincipledMaterial::setVertexColorsMaskEnabled(bool vertexColorsMaskEnabled)
{
    if (m_vertexColorsMaskEnabled == vertexColorsMaskEnabled)
        return;
    m_vertexColorsMaskEnabled = vertexColorsMaskEnabled;
    emit vertexColorsMaskEnabledChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DPrincipledMaterial::VertexColorMaskFlags QQuick3DPrincipledMaterial::vertexColorRedMask() const
{
    return m_vertexColorRedMask;
}

void QQuick3DPrincipledMaterial::setVertexColorRedMask(QQuick3DPrincipledMaterial::VertexColorMaskFlags vertexColorRedMask)
{
    if (m_vertexColorRedMask == vertexColorRedMask)
        return;
    m_vertexColorRedMask = vertexColorRedMask;
    emit vertexColorRedMaskChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DPrincipledMaterial::VertexColorMaskFlags QQuick3DPrincipledMaterial::vertexColorGreenMask() const
{
    return m_vertexColorGreenMask;
}

void QQuick3DPrincipledMaterial::setVertexColorGreenMask(QQuick3DPrincipledMaterial::VertexColorMaskFlags vertexColorGreenMask)
{
    if (m_vertexColorGreenMask == vertexColorGreenMask)
        return;
    m_vertexColorGreenMask = vertexColorGreenMask;
    emit vertexColorGreenMaskChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DPrincipledMaterial::VertexColorMaskFlags QQuick3DPrincipledMaterial::vertexColorBlueMask() const
{
    return m_vertexColorBlueMask;
}

void QQuick3DPrincipledMaterial::setVertexColorBlueMask(QQuick3DPrincipledMaterial::VertexColorMaskFlags vertexColorBlueMask)
{
    if (m_vertexColorBlueMask == vertexColorBlueMask)
        return;
    m_vertexColorBlueMask = vertexColorBlueMask;
    emit vertexColorBlueMaskChanged();
    markDirty(VertexColorsDirty);
}

QQuick3DPrincipledMaterial::VertexColorMaskFlags QQuick3DPrincipledMaterial::vertexColorAlphaMask() const
{
    return m_vertexColorAlphaMask;
}

void QQuick3DPrincipledMaterial::setVertexColorAlphaMask(QQuick3DPrincipledMaterial::VertexColorMaskFlags vertexColorAlphaMask)
{
    if (m_vertexColorAlphaMask == vertexColorAlphaMask)
        return;
    m_vertexColorAlphaMask = vertexColorAlphaMask;
    emit vertexColorAlphaMaskChanged();
    markDirty(VertexColorsDirty);
}

QT_END_NAMESPACE


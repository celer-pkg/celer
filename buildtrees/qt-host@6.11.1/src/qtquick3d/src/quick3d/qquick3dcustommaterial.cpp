// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dcustommaterial_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <ssg/qssgrendercontextcore.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>
#include <QtQuick/QQuickWindow>

#include <QtQuick3D/QQuick3DTextureProviderExtension>

#include "qquick3dobject_p.h"
#include "qquick3dviewport_p.h"
#include "qquick3dscenemanager_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype CustomMaterial
    \inherits Material
    \inqmlmodule QtQuick3D
    \brief Base component for creating custom materials used to shade models.

    The custom material allows using custom shader code for a material, enabling
    programmability on graphics shader level. A vertex, fragment, or both
    shaders can be provided. The \l vertexShader and \l fragmentShader
    properties are URLs, referencing files containing shader snippets, and work
    very similarly to ShaderEffect or \l{Image::source}{Image.source}. Only the
    \c file and \c qrc schemes are supported with custom materials. It is also
    possible to omit the \c file scheme, allowing to specify a relative path in
    a convenient way. Such a path is resolved relative to the component's (the
    \c{.qml} file's) location.

    For a getting started guide to custom materials, see the page \l{Programmable
    Materials, Effects, Geometry, and Texture data}.

    \section1 Introduction

    Consider the following versions of the same scene. On the left, the cylinder
    is using a built-in, non-programmable material. Such materials are
    configurable through a wide range of properties, but there is no further
    control given over the shaders that are generated under the hood. On the
    right, the same cylinder is now associated with a CustomMaterial referencing
    application-provided vertex and fragment shader snippets. This allows
    inserting custom, application-specific logic into the vertex shader to
    transform the geometry, and to determine certain color properties in a
    custom manner in the fragment shader. As this is a
    \l{shadingMode}{shaded} custom material, the cylinder still
    participates in the scene lighting normally.

    \table 70%
    \row
    \li \qml
    View3D {
        anchors.fill: parent
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }
        camera: camera
        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }
        Model {
            source: "#Cylinder"
            eulerRotation: Qt.vector3d(30, 30, 0)
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            materials: [
                DefaultMaterial {
                    diffuseColor: Qt.rgba(0, 1, 0, 1)
                }
            ]
        }
    }
        \endqml
    \li \qml
    View3D {
        anchors.fill: parent
        PerspectiveCamera {
            id: camera
            position: Qt.vector3d(0, 0, 600)
        }
        camera: camera
        DirectionalLight {
            position: Qt.vector3d(-500, 500, -100)
            color: Qt.rgba(0.2, 0.2, 0.2, 1.0)
            ambientColor: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        }
        Model {
            source: "#Cylinder"
            eulerRotation: Qt.vector3d(30, 30, 0)
            scale: Qt.vector3d(1.5, 1.5, 1.5)
            materials: [
                CustomMaterial {
                    vertexShader: "material.vert"
                    fragmentShader: "material.frag"
                    property real uTime
                    property real uAmplitude: 50
                    NumberAnimation on uTime { from: 0; to: 100; duration: 10000; loops: -1 }
                }
            ]
        }
    }
        \endqml
    \endtable

    Let's assume that the shader snippets in \c{material.vert} and \c{material.frag} are
    the following:

    \table 70%
    \row
    \li \badcode
    void MAIN()
    {
        VERTEX.x += sin(uTime + VERTEX.y) * uAmplitude;
    }
    \endcode
    \li \badcode
    void MAIN()
    {
        BASE_COLOR = vec4(0.0, 1.0, 0.0, 1.0);
    }
    \endcode
    \endtable

    Notice how \c uTime and \c uAmplitude are properties of the CustomMaterial
    element. They can change values and get animated normally, the values will
    be exposed to the shaders automatically without any further action from the
    developer.

    The result is a cylinder that animates its vertices:

    \image custommaterial_cylinder.png
           {Bent cylinder with custom vertex shader}

    \section1 Two flavors of custom materials

    There are two main types of custom materials. This is specified by the \l
    shadingMode property. In \l{CustomMaterial::shadingMode}{unshaded} custom
    materials the fragment shader outputs a single \c vec4 color, ignoring
    lights, light probes, shadowing in the scene. In
    \l{CustomMaterial::shadingMode}{shaded} materials the shader is expected to
    implement certain functions and work with built-in variables to take
    lighting and shadow contribution into account.

    The default choice is typically a shaded material, this is reflected in the
    default value of the \l shadingMode property. This fits materials that needs
    to transform vertices or other incoming data from the geometry, or determine
    values like \c BASE_COLOR or \c EMISSIVE_COLOR in a custom manner, perhaps
    by sampling \c SCREEN_TEXTURE or \c DEPTH_TEXTURE, while still reciving
    light and shadow contributions from the scene. Additionally, such materials
    can also override and reimplement the equations used to calculate the
    contributions from directional, point, and other lights. The
    application-provided shader snippets are heavily amended by the Qt Quick 3D
    engine under the hood, in order to provide the features, such as lighting,
    the standard materials have.

    Unshaded materials are useful when the object's appearance is determined
    completely by the custom shader code.  The shaders for such materials
    receive minimal additions by the engine, and therefore it is completely up
    to the shader to determine the final fragment color. This gives more
    freedom, but also limits possiblities to integrate with other elements of
    the scene, such as lights.

    \note Shader code is always provided using Vulkan-style GLSL, regardless of
    the graphics API used by Qt at run time.

    \note The vertex and fragment shader code provided by the material are not
    full, complete GLSL shaders on their own. Rather, they provide a set of
    functions, which are then amended with further shader code by the engine.

    \section1 Exposing data to the shaders

    The dynamic properties of the CustomMaterial can be changed and animated
    using QML and Qt Quick facilities, and the values are exposed to the
    shaders automatically. This in practice is very similar ShaderEffect. The
    following list shows how properties are mapped:

    \list
    \li bool, int, real -> bool, int, float
    \li QColor, \l{QtQml::Qt::rgba()}{color} -> vec4, and the color gets
    converted to linear, assuming sRGB space for the color value specified in
    QML. The built-in Qt colors, such as \c{"green"} are in sRGB color space as
    well, and the same conversion is performed for all color properties of
    DefaultMaterial and PrincipledMaterial, so this behavior of CustomMaterial
    matches those. Unlike Qt Quick, for Qt Quick 3D linearizing is essential as
    there will typically be tonemapping performed on the 3D scene.
    \li QRect, QRectF, \l{QtQml::Qt::rect()}{rect} -> vec4
    \li QPoint, QPointF, \l{QtQml::Qt::point()}{point}, QSize, QSizeF, \l{QtQml::Qt::size()}{size} -> vec2
    \li QVector2D, \l{QtQml::Qt::vector2d()}{vector2d} -> vec2
    \li QVector3D, \l{QtQml::Qt::vector3d()}{vector3d} -> vec3
    \li QVector4D, \l{QtQml::Qt::vector4d()}{vector4d} -> vec4
    \li QMatrix4x4, \l{QtQml::Qt::matrix4x4()}{matrix4x4} -> mat4
    \li QQuaternion, \l{QtQml::Qt::quaternion()}{quaternion} -> vec4, scalar value is \c w

    \li TextureInput -> sampler2D or samplerCube, depending on whether \l
    Texture or \l CubeMapTexture is used in the texture property of the
    TextureInput. Setting the \l{TextureInput::enabled}{enabled} property to
    false leads to exposing a dummy texture to the shader, meaning the shaders
    are still functional but will sample a texture with opaque black image
    content. Pay attention to the fact that properties for samplers must always
    reference a \l TextureInput object, not a \l Texture directly. When it
    comes to the \l Texture properties, the source, tiling, and filtering
    related ones are the only ones that are taken into account implicitly with
    custom materials, as the rest (such as, UV transformations) is up to the
    custom shaders to implement as they see fit.

    \endlist

    \note When a uniform referenced in the shader code does not have a
    corresponding property, it will cause a shader compilation error when
    processing the material at run time. There are some exceptions to this,
    such as, sampler uniforms, that get a dummy texture bound when no
    corresponding QML property is present, but as a general rule, all uniforms
    and samplers must have a corresponding property declared in the
    CustomMaterial object.

    \section1 Unshaded custom materials

    The following is an example of an \l{CustomMaterial::shadingMode}{unshaded}
    custom material.

    \qml
    CustomMaterial {
        // These properties are automatically exposed to the shaders
        property real time: 0.0
        property real amplitude: 5.0
        property real alpha: 1.0
        property TextureInput tex: TextureInput {
            enabled: true
            texture: Texture { source: "image.png" }
        }

        shadingMode: CustomMaterial.Unshaded
        sourceBlend: alpha < 1.0 ? CustomMaterial.SrcAlpha : CustomMaterial.NoBlend
        destinationBlend: alpha < 1.0 ? CustomMaterial.OneMinusSrcAlpha : CustomMaterial.NoBlend
        cullMode: CustomMaterial.BackFaceCulling

        vertexShader: "customshader.vert"
        fragmentShader: "customshader.frag"
    }
    \endqml

    With the above example, the \l{CustomMaterial::shadingMode}{unshaded} vertex
    and fragment shaders snippets could look like the following. Note how the
    shaders do not, and must not, declare uniforms or vertex inputs as that is
    taken care of by Qt when assembling the final shader code.

    \badcode
    VARYING vec3 pos;
    VARYING vec2 texcoord;

    void MAIN()
    {
        pos = VERTEX;
        pos.x += sin(time * 4.0 + pos.y) * amplitude;
        texcoord = UV0;
        POSITION = MODELVIEWPROJECTION_MATRIX * vec4(pos, 1.0);
    }
    \endcode

    \badcode
    VARYING vec3 pos;
    VARYING vec2 texcoord;

    void MAIN()
    {
        vec4 c = texture(tex, texcoord);
        FRAGCOLOR = vec4(pos.x * 0.02, pos.y * 0.02, pos.z * 0.02, alpha) * c;
    }
    \endcode

    The following special, uppercase keywords are available:

    \list

    \li MAIN -> the name of the entry point in the vertex or fragment shader
    snippet must always be \c MAIN. Providing this function is mandatory in
    shader snippets for unshaded custom materials.

    \li VARYING -> declares an output from the vertex shader or an input to the
    fragment shader

    \li POSITION -> vec4, the output from the vertex shader

    \li FRAGCOLOR -> vec4, the output from the fragment shader. Available only
    for unshaded custom materials.

    \li VERTEX -> vec3, the vertex position in the vertex shader.

    \li NORMAL -> vec3, the vertex normal in the vertex shader. When the mesh
    for the associated model does not provide normals, the value is vec3(0.0).

    \li UV0 -> vec2, the first set of texture coordinates in the vertex shader.
    When the mesh for the associated model does not provide texture
    coordinates, the value is vec2(0.0).

    \li UV1 -> vec2, the second set of texture coordinates in the vertex
    shader. When the mesh for the associated model does not provide a second
    set of texture coordinates, the value is vec2(0.0).

    \li COLOR -> vec4, the vertex color in the vertex shader. When the mesh for
    the associated model does not provide per-vertex colors, the value is
    vec4(1.0).

    \li TANGENT -> vec3, tangent in the vertex shader. When the mesh for the
    associated model does not provide tangent data, the value is vec3(0.0).

    \li BINORMAL -> vec3, binormal in the vertex shader. When the mesh for the
    associated model does not provide binormal data, the value is vec3(0.0).

    \li JOINTS -> ivec4, joint indexes in the vertex shader. When the mesh for
    the associated model does not provide joint indexes data, the value is
    ivec4(0).

    \li WEIGHTS -> vec4, joint weights in the vertex shader. When the mesh for
    the associated model does not provide joint weights data, the value is
    vec4(0.0).

    \li MORPH_POSITION(\e{n}) -> vec3, the \e{n+1}th morph target position in the vertex
    shader. The associated model should provide proper data.

    \li MORPH_NORMAL(\e{n}) -> vec3, the \e{n+1}th morph target normal in the vertex
    shader. The associated model should provide proper data.

    \li MORPH_TANGENT(\e{n}) -> vec3, the \e{n+1}th morph target tangent in the vertex
    shader. The associated model should provide proper data.

    \li MORPH_BINORMAL(\e{n}) -> vec3, the \e{n+1}th morph target binormal in the vertex
    shader. The associated model should provide proper data.

    \li MODELVIEWPROJECTION_MATRIX -> mat4, the model-view-projection matrix.
    Projection matrices always follow OpenGL conventions, with a baked-in
    transformation for the Y axis direction and clip depth, depending on the
    graphics API used at run time.

    \li VIEWPROJECTION_MATRIX -> mat4, the view-projection matrix

    \li PROJECTION_MATRIX -> mat4, the projection matrix

    \li INVERSE_PROJECTION_MATRIX -> mat4, the inverse projection matrix

    \li VIEW_MATRIX -> mat4, the view (camera) matrix

    \li MODEL_MATRIX -> mat4, the model (world) matrix

    \li NORMAL_MATRIX -> mat3, the normal matrix (the transpose of the inverse
    of the top-left 3x3 part of the model matrix)

    \li BONE_TRANSFORMS -> mat4[], the array of the model's bone matrixes

    \li BONE_NORMAL_TRANSFORMS -> mat3[], the array of the model's bone normal
    matrixes (the transpose of the inverse of the top-left 3x3 part of the each
    bone matrixes)

    \li MORPH_WEIGHTS -> float[], the array of the morph weights. The associated model
    should provide proper data. For safety, \b {QT_MORPH_MAX_COUNT} is defined to the
    size of this array.

    \li CAMERA_POSITION -> vec3, the camera position in world space

    \li CAMERA_DIRECTION -> vec3, the camera direction vector

    \li CAMERA_PROPERTIES -> vec2, the near and far clip values for the camera

    \li POINT_SIZE -> float, writable in the vertex shader only. When rendering
    geometry with a topology of points, the custom vertex shader must set this
    to either 1.0 or another value, both in shaded and unshaded custom
    materials. See \l{PrincipledMaterial::pointSize} for further notes on
    support for sizes other than 1.

    \endlist

    \section1 Shaded custom materials

    A \l{CustomMaterial::shadingMode}{shaded} material \c augments the shader code
    that would be generated by a PrincipledMaterial. Unlike unshaded materials,
    that provide almost all logic for the vertex and fragment shader main
    functions on their own, preventing adding generated code for lighting,
    shadowing, global illumination, etc., shaded materials let shader
    generation happen normally, as if the CustomMaterial was a
    PrincipledMaterial. The vertex and fragment shader snippets are expected to
    provide optional functions that are then invoked at certain points, giving
    them the possibility to customize the colors and other values that are then
    used for calculating lighting and the final fragment color.

    Rather than implementing just a \c MAIN function, the fragment shader for a
    shaded custom material can implement multiple functions. All functions,
    including \c MAIN, are optional to implement in shaded custom materials. An
    empty shader snippet, or, even, not specifying the
    \l{CustomMaterial::vertexShader}{vertexShader} or
    \l{CustomMaterial::fragmentShader}{fragmentShader} properties at all can be
    perfectly valid too.

    \section2 Vertex shader snippets in a shaded custom material

    The following functions can be implemented in a vertex shader snippet:

    \list

    \li \c{void MAIN()} When present, this function is called in order to set
    the value of \c POSITION, the vec4 output from the vertex shader, and,
    optionally, to modify the values of \c VERTEX, \c COLOR, \c NORMAL, \c UV0,
    \c UV1, \c TANGENT, \c BINORMAL, \c JOINTS, and \c WEIGHTS. Unlike in
    unshaded materials, writing to these makes sense because the modified values
    are then taken into account in the rest of the generated shader code
    (whereas for unshaded materials there is no additional shader code
    generated). For example, if the custom vertex shader displaces the vertices
    or the normals, it will want to store the modified values to \c VERTEX or
    \c NORMAL, to achieve correct lighting calculations afterwards.
    Additionally, the function can write to variables defined with \c VARYING in
    order to pass interpolated data to the fragment shader. When this function
    or a redefinition of \c POSITION is not present, \c POSITION is calculated
    based on \c VERTEX and \c MODELVIEWPROJECTION_MATRIX, just like a
    PrincipledMaterial would do.

    Example, with relying both on QML properties exposed as uniforms, and also
    passing data to the fragment shader:
    \badcode
        VARYING vec3 vNormal;
        VARYING vec3 vViewVec;

        void MAIN()
        {
            VERTEX.x += sin(uTime * 4.0 + VERTEX.y) * uAmplitude;
            vNormal = normalize(NORMAL_MATRIX * NORMAL);
            vViewVec = CAMERA_POSITION - (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
            POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
        }
    \endcode

    \note In the above example, assigning a value to \c POSITION is optional as
    the usage in this case is identical to the default behavior.

    \endlist

    \note To pass data without interpolation from the vertex to the fragment
    stage, add the \c flat keyword before the type in the \c VARYING
    declarations.

    \section2 Fragment shader snippets in a shaded custom material

    The following functions can be implemented in a fragment shader snippet:

    \list

    \li \c{void MAIN()} When present, this function is called to set the values
    of the special writable variables \c BASE_COLOR, \c METALNESS, \c ROUGHNESS, \c
    SPECULAR_AMOUNT, NORMAL, CLEARCOAT_FRESNEL_POWER, CLEARCOAT_FRESNEL_SCALE,
    CLEARCOAT_FRESNEL_BIAS, CLEARCOAT_AMOUNT, CLEARCOAT_ROUGHNESS, CLEARCOAT_NORMAL,
    FRESNEL_BIAS, FRESNEL_SCALE, FRESNEL_POWER, IOR, \c TRANSMISSION_FACTOR,
    THICKNESS_FACTOR, ATTENUATION_COLOR, ATTENUATION_DISTANCE and \c OCCLUSION_AMOUNT.

    One common use case is to set the value of \c BASE_COLOR based on sampling
    a texture, be it a base color map, \c SCREEN_TEXTURE, or some other kind of
    source. This can be relevant and convenient especially when no custom light
    processor functions are implemented. Setting \c{BASE_COLOR.a} to something
    other than the default 1.0 allows affecting the final alpha value of the
    fragment. (note that this will often require also enabling alpha blending
    in \l sourceBlend and \l destinationBlend)

    Another scenario is when there is no custom \c SPECULAR_LIGHT function
    provided, or when there is a light probe set in the SceneEnvironment. The
    metalness, roughness, and other values that affect the specular
    contribution calculation can be set in \c MAIN to their desired custom
    values.

    The function can write to the following special variables. The values
    written to these will typically be either hardcoded or be calculated based
    on QML properties mapped to uniforms. The semantics are identical to
    PrincipledMaterial.

    \list

    \li vec4 \c BASE_COLOR - The base color and material alpha value.
    Corresponds to the \l{PrincipledMaterial::baseColor}{built-in materials'
    color property}. When light processor functions are not implemented, it can
    be convenient to set a custom base color in \c MAIN because that is then
    taken into account in the default lighting calculations. The default value
    is \c{vec4(1.0)}, meaning white with an alpha of 1.0. The alpha value
    effects the final alpha of the fragment. The final alpha value is the
    object (model) opacity multiplied by the base color alpha. When specifying
    the value directly in shader code, not relying on uniform values exposed
    from \b color properties in QML, be aware that it is up to the shader to
    perform the sRGB to linear conversion, if needed. For example, assuming
    a \c{vec3 color} and \c{float alpha} this can be achieved like the following:
    \badcode
    float C1 = 0.305306011;
    vec3 C2 = vec3(0.682171111, 0.682171111, 0.682171111);
    vec3 C3 = vec3(0.012522878, 0.012522878, 0.012522878);
    BASE_COLOR = vec4(rgb * (rgb * (rgb * C1 + C2) + C3), alpha);
    \endcode

    \li vec3 \c EMISSIVE_COLOR - The color of self-illumination. Corresponds to
    the built-in materials' emissive color which is combined by
    \l {PrincipledMaterial::emissiveFactor}{built-in materials's emissiveFactor property}
    and \l {PrincipledMaterial::emissiveMap}{built-in materials's emissiveMap property}.
    The default value is \c{vec3(0.0)}. When specifying the value
    directly in shader code, not relying on uniform values exposed from \b color
    properties in QML, be aware that it is up to the shader to perform the sRGB
    to linear conversion, if needed.

    \li float \c IOR Specifies the index of refraction of the material. A typical value,
    and also the default, is \c{1.5} as that is what a PrincipledMaterial would use.

    \li float \c TRANSMISSION_FACTOR Specifies the amount of the translucency. A typical value,
    would be \c{1.0} and also the default, is \c{0.0} as that is what a PrincipledMaterial would use.

    \li float \c THICKNESS_FACTOR Specifies the amount of the translucent material thickness. A typical value,
    would be \c{10.0} and also the default, is \c{0.0} as that is what a PrincipledMaterial would use.

    \li vec3 \c ATTENUATION_COLOR Specifies the color shift of the translucent material by distance. A typical value,
    would be \c{vec3(1.0, 0.0, 0.0)} and also the default, is \c{vec3(1.0)} as that is what a PrincipledMaterial would use.

    \li float \c ATTENUATION_DISTANCE Specifies the distance attenuation of color shift of the translucent material. A typical value,
    would be \c{100.0} and also the default, is \c{0.0} as that is what a PrincipledMaterial would use.

    \li float \c METALNESS Metalness amount in range 0.0 - 1.0. The default
    value is 0. Must be set to a non-zero value to have effect.

    \li float \c ROUGHNESS Roughness value in range 0.0 - 1.0. The default value is 0.

    \li float \c CLEARCOAT_FRESNEL_POWER Specifies the fresnel power of the clearcoat layer. A typical value,
    and also the default, is \c{5.0} as that is what a PrincipledMaterial would use.

    \li float \c CLEARCOAT_FRESNEL_SCALE Specifies the fresnel scale of the clearcoat layer. A typical value,
    and also the default, is \c{1.0} as that is what a PrincipledMaterial would use.

    \li float \c CLEARCOAT_FRESNEL_BIAS Specifies the fresnel bias of the clearcoat layer. A typical value,
    and also the default, is \c{0.0} as that is what a PrincipledMaterial would use.

    \li float \c CLEARCOAT_AMOUNT Specifies the amount of the clearcoat layer on top of the material. A typical value,
    would be \c{1.0} and also the default, is \c{0.0} as that is what a PrincipledMaterial would use.

    \li float \c CLEARCOAT_ROUGHNESS Specifies the roughness of the clearcoat layer. A typical value,
    would be \c{1.0} for fully blurred clearcoat layer and also the default, is \c{0.0} as that is
    what a PrincipledMaterial would use.

    \li vec3 \c CLEARCOAT_NORMAL - The clearcoat layer normal that comes from the vertex shader in world
    space. While this property has the same initial value as \c VAR_WORLD_NORMAL,
    only changing the value of \c CLEARCOAT_NORMAL will have an effect on clearcoat layer normal.

    \li float \c FRESNEL_POWER Specifies the fresnel power. A typical value,
    and also the default, is \c{5.0} as that is what a PrincipledMaterial would use.

    \li float \c FRESNEL_SCALE Specifies the fresnel scale. A typical value,
    and also the default, is \c{1.0} as that is what a PrincipledMaterial would use.

    \li float \c FRESNEL_BIAS Specifies the fresnel bias. A typical value,
    and also the default, is \c{0.0} as that is what a PrincipledMaterial would use.

    \li float \c SPECULAR_AMOUNT Specular amount in range 0.0 - 1.0. The
    default value is \c{0.5}, matching \l{PrincipledMaterial::specularAmount}. Must
    be set to a non-zero value to have effect.

    \li float \c OCCLUSION_AMOUNT Specifies the AO factor. A typical value,
    and also the default, is \c{1.0} as that is what a PrincipledMaterial would use.

    \li vec3 \c NORMAL - The normal that comes from the vertex shader in world
    space. While this property has the same initial value as \c VAR_WORLD_NORMAL,
    only changing the value of \c NORMAL will have an effect on lighting.

    \li vec3 \c TANGENT - The tanget that comes from the vertex shader in world
    space. This value is potentially adjusted for double-sidedness.

    \li vec3 \c BINORMAL - The binormal that comes from the vertex shader in
    world space. This value is potentially adjusted for double-sidedness.

    \li vec2 \c UV0 - The first set of texture coordinates from the vertex shader.
    This property is readonly in the fragment shader.

    \li vec2 \c UV1 - The second set of texture coordinates from the vertex shader.
    This property is readonly in the fragment shader.

    \endlist

    \note Unlike with unshaded materials, the fragment \c MAIN for a shaded
    material has no direct control over \c FRAGCOLOR. Rather, it is the \c
    DIFFUSE and \c SPECULAR values written in the light processor functions
    that decide what the final fragment color is. When a light processor
    function is not implemented, the relevant default shading calculations are
    performed as with a PrincipledMaterial, taking \c BASE_COLOR and other
    values from the list above into account.

    An example of a simple, metallic custom material shader could be the following:
    \badcode
        void MAIN()
        {
            METALNESS = 1.0;
            ROUGHNESS = 0.5;
            FRESNEL_POWER = 5.0;
        }
    \endcode

    Another example, where the base color and alpha are set by sampling a texture:
    \badcode
        VARYING vec2 texcoord;
        void MAIN()
        {
            BASE_COLOR = texture(uColorMap, texcoord);
        }
    \endcode

    \li \c{void AMBIENT_LIGHT()} When present, this function is called once for
    each fragment. The task of the function is to add the total ambient
    contribution to a writable special variable \c DIFFUSE. It can of course
    choose to calculate a different value, or not touch \c DIFFUSE at all (to
    ignore ambient lighting completely). When this function is not present at
    all, the ambient contribution is calculated normally, like a
    PrincipledMaterial would do.

    The function can write to the following special variables:

    \list

    \li vec3 \c DIFFUSE Accumulates the diffuse light contributions, per
    fragment. The light processor functions will typically add (\c{+=}) to it,
    since overwriting the value would lose the contribution from other lights.

    \endlist

    The function can read the following special variables, in addition to the
    matrix (such as, \c MODEL_MATRIX) and vector (such as, \c CAMERA_POSITION)
    uniforms from the table above:

    \list
    \li vec3 \c TOTAL_AMBIENT_COLOR The total ambient contribution in the scene.
    \endlist

    Example:
    \badcode
        void AMBIENT_LIGHT()
        {
            DIFFUSE += TOTAL_AMBIENT_COLOR;
        }
    \endcode

    \li \c{void DIRECTIONAL_LIGHT()} When present, this function is called for
    each active directional light in the scene for each fragment. The task of
    the function is to add the diffuse contribution to a writable special
    variable \c DIFFUSE. The function can also choose to do nothing, in which
    case diffuse contributions from directional lights are ignored. When the
    function is not present at all, the diffuse contributions from directional
    lights are accumulated normally, like a PrincipledMaterial would do.

    The function can write to the following special variables:

    \list

    \li vec3 \c DIFFUSE Accumulates the diffuse light contributions, per
    fragment. The light processor functions will typically add (\c{+=}) to it,
    since overwriting the value would lose the contribution from other lights.

    \endlist

    The function can read the following special variables, in addition to the
    matrix (such as, \c MODEL_MATRIX) and vector (such as, \c CAMERA_POSITION)
    uniforms from the table above:

    \list

    \li vec3 \c LIGHT_COLOR Diffuse light color.
    \li float \c SHADOW_CONTRIB Shadow contribution, or 1.0 if not shadowed at all or not reciving shadows.
    \li vec3 \c TO_LIGHT_DIR Vector pointing towards the light source.
    \li vec3 \c NORMAL The normal vector in world space.
    \li vec4 \c BASE_COLOR The base color and material alpha value.
    \li float \c METALNESS The Metalness amount.
    \li float \c ROUGHNESS The Roughness amount.

    \endlist

    Example:
    \badcode
        void DIRECTIONAL_LIGHT()
        {
            DIFFUSE += LIGHT_COLOR * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(VAR_WORLD_NORMAL), TO_LIGHT_DIR)));
        }
    \endcode

    \li \c{void POINT_LIGHT()} When present, this function is called for
    each active point light in the scene for each fragment. The task of
    the function is to add the diffuse contribution to a writable special
    variable \c DIFFUSE. The function can also choose to do nothing, in which
    case diffuse contributions from point lights are ignored. When the
    function is not present at all, the diffuse contributions from point
    lights are accumulated normally, like a PrincipledMaterial would do.

    The function can write to the following special variables:

    \list
    \li vec3 \c DIFFUSE Accumulates the diffuse light contributions, per fragment.
    \endlist

    The function can read the following special variables, in addition to the
    matrix (such as, \c MODEL_MATRIX) and vector (such as, \c CAMERA_POSITION)
    uniforms from the table above:

    \list
    \li vec3 \c LIGHT_COLOR Diffuse light color.
    \li float \c LIGHT_ATTENUATION Light attenuation.
    \li float \c SHADOW_CONTRIB Shadow contribution, or 1.0 if not shadowed at all or not reciving shadows.
    \li vec3 \c TO_LIGHT_DIR Vector pointing towards the light source.
    \li vec3 \c NORMAL The normal vector in world space.
    \li vec4 \c BASE_COLOR The base color and material alpha value.
    \li float \c METALNESS The Metalness amount.
    \li float \c ROUGHNESS The Roughness amount.
    \endlist

    Example:
    \badcode
        void POINT_LIGHT()
        {
            DIFFUSE += LIGHT_COLOR * LIGHT_ATTENUATION * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(VAR_WORLD_NORMAL), TO_LIGHT_DIR)));
        }
    \endcode

    \li \c{void SPOT_LIGHT()} When present, this function is called for
    each active spot light in the scene for each fragment. The task of
    the function is to add the diffuse contribution to a writable special
    variable \c DIFFUSE. The function can also choose to do nothing, in which
    case diffuse contributions from spot lights are ignored. When the
    function is not present at all, the diffuse contributions from spot
    lights are accumulated normally, like a PrincipledMaterial would do.

    The function can write to the following special variables:

    \list
    \li vec3 \c DIFFUSE Accumulates the diffuse light contributions, per fragment.
    \endlist

    The function can read the following special variables, in addition to the
    matrix (such as, \c MODEL_MATRIX) and vector (such as, \c CAMERA_POSITION)
    uniforms from the table above:

    \list
    \li vec3 \c LIGHT_COLOR Diffuse light color.
    \li float \c LIGHT_ATTENUATION Light attenuation.
    \li float \c SHADOW_CONTRIB Shadow contribution, or 1.0 if not shadowed at all or not reciving shadows.
    \li vec3 \c TO_LIGHT_DIR Vector pointing towards the light source.
    \li float \c SPOT_FACTOR Spot light factor.
    \li vec3 \c NORMAL The normal vector in world space.
    \li vec4 \c BASE_COLOR The base color and material alpha value.
    \li float \c METALNESS The Metalness amount.
    \li float \c ROUGHNESS The Roughness amount.
    \endlist

    Example:
    \badcode
        void SPOT_LIGHT()
        {
            DIFFUSE += LIGHT_COLOR * LIGHT_ATTENUATION * SPOT_FACTOR * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(VAR_WORLD_NORMAL), TO_LIGHT_DIR)));
        }
    \endcode

    \li \c{void SPECULAR_LIGHT()} When present, this function is called for
    each active light in the scene for each fragment. The task of the function
    is to add the specular contribution to a writable special variable \c
    SPECULAR. The function can also choose to do nothing, in which case
    specular contributions from lights are ignored. When the function is not
    present at all, the specular contributions from lights are accumulated
    normally, like a PrincipledMaterial would do.

    The function can write to the following special variables:

    \list

    \li vec3 \c SPECULAR Accumulates the specular light contributions, per
    frament. The light processor functions will typically add (\c{+=}) to it,
    since overwriting the value would lose the contribution from other lights.

    \endlist

    The function can read the following special variables, in addition to the
    matrix (such as, \c MODEL_MATRIX) and vector (such as, \c CAMERA_POSITION)
    uniforms from the table above:

    \list
    \li vec3 \c LIGHT_COLOR Specular light color.
    \li float \c LIGHT_ATTENUATION Light attenuation. For directional lights the value is 1.0. For spot lights the value is the same as \c {LIGHT_ATTENUATION * SPOT_FACTOR} of \c {void SPOT_LIGHT()}.
    \li float \c SHADOW_CONTRIB Shadow contribution, or 1.0 if not shadowed at all or not reciving shadows.
    \li vec3 \c FRESNEL_CONTRIB Fresnel contribution from built in Fresnel calculation.
    \li vec3 \c TO_LIGHT_DIR Vector pointing towards the light source.
    \li vec3 \c NORMAL The normal vector in world space.
    \li vec4 \c BASE_COLOR The base color and material alpha value.
    \li float \c METALNESS The Metalness amount.
    \li float \c ROUGHNESS The Roughness amount.
    \li float \c SPECULAR_AMOUNT The specular amount. This value will be between
    0.0 and 1.0 will be the same value set in the custom \c MAIN function. This
    value will useful for calculating Fresnel contributions when not using the
    built-in Fresnel contribution provided by \c FRESNEL_CONTRIB.
    \endlist

    \badcode
        void SPECULAR_LIGHT()
        {
            vec3 H = normalize(VIEW_VECTOR + TO_LIGHT_DIR);
            float cosAlpha = max(0.0, dot(H, normalize(NORMAL)));
            float shine = pow(cosAlpha, exp2(15.0 * (1.0 - ROUGHNESS) + 1.0) * 0.25);
            SPECULAR += shine * LIGHT_COLOR * FRESNEL_CONTRIB * SHADOW_CONTRIB * LIGHT_ATTENUATION;
        }
    \endcode

    \li \c{void POST_PROCESS()} When present, this function is called at the
    end of the fragment pipeline. The task of the function is to finalize
    \c COLOR_SUM with final diffuse, specular and emissive terms. Unlike
    \c FRAGCOLOR for a unshaded material, \c COLOR_SUM will be automatically
    tonemapped before written to the framebuffer. For debugging purposes it is
    sometimes useful to output a value that should not be treated as a color.
    To avoid the tonemapping distorting this value it can be disabled by
    setting the \l {SceneEnvironment::tonemapMode}{tonemapMode} property
    to \c TonemapModeNone

    The function can write to the following special variables:

    \list
    \li vec4 \c COLOR_SUM the output from the fragment shader. The default value
    is vec4(DIFFUSE.rgb + SPECULAR + EMISSIVE, DIFFUSE.a)
    \endlist

    The function can read the following special variables.

    \list
    \li vec4 \c DIFFUSE The final diffuse term of the fragment pipeline.
    \li vec3 \c SPECULAR The final specular term of the fragment pipeline.
    \li vec3 \c EMISSIVE The final emissive term of the fragment pipeline.
    \li vec2 \c UV0 - The first set of texture coordinates from the vertex shader.
    \li vec2 \c UV1 - The second set of texture coordinates from the vertex shader.
    \endlist

    \badcode
        void POST_PROCESS()
        {
            float center_x = textureSize(SCREEN_TEXTURE, 0).x * 0.5;
            if (gl_FragCoord.x > center_x)
                COLOR_SUM = DIFFUSE;
            else
                COLOR_SUM = vec4(EMISSIVE, DIFFUSE.a);
        }
    \endcode

    \li \c{void IBL_PROBE()} When present, this function is called for IBL
    (Image-Based Lighting).
    The task of the function is to add both the diffuse and the specular
    contributions of IBL to writable special variables \c DIFFUSE and
    \c SPECULAR.

    The function can write to the following special variables:

    \list
    \li vec3 \c DIFFUSE Accumulates the diffuse light contributions, per fragment.
    \li vec3 \c SPECULAR Accumulates the specular light contributions, per
    frament.
    \endlist

    The function can read the following special variables.

    \list
    \li vec4 \c BASE_COLOR The base color and material alpha value.
    \li float \c AO_FACTOR The screen space occlusion factor.
    \li float \c SPECULAR_AMOUNT The specular amount.
    \li float \c ROUGHNESS The final emissive term of the fragment pipeline.
    \li vec3 \c NORMAL The normal vector in world space.
    \li vec3 \c VIEW_VECTOR Points towards the camera.
    \li mat3 \c IBL_ORIENTATION The orientation of the light probe. It comes
    from \l {SceneEnvironment::probeOrientation}.
    \endlist

    \badcode
        void IBL_PROBE()
        {
            vec3 smpDir = IBL_ORIENTATION * NORMAL;
            DIFFUSE += AO_FACTOR * BASE_COLOR.rgb * textureLod(IBL_TEXTURE, smpDir, IBL_MAXMIPMAP).rgb;
        }
    \endcode

    \endlist

    \sa SceneEnvironment::tonemapMode, {Using Image-Based Lighting}

    \section2 Custom variables between functions

    Additional variables can be delivered from the MAIN function to the others.
    The \c SHARED_VARS keyword can be used for defining new custom variables.
    These user-defined variables can be accessed with SHARED.<variable name>.

    For example, a shaded custom material can fetch a shared value in the MAIN
    and use it in other functions.

    \badcode
        SHARED_VARS {
            vec3 colorThreshold;
        };
        void MAIN()
        {
            BASE_COLOR = texture(baseColorMap, UV0);
            SHARED.colorThreshold = texture(thresholdMap, UV0).rgb;
        }
        void DIRECTIONAL_LIGHT()
        {
            if (DIFFUSE >= SHARED.colorThreshold) {
                DIFFUSE = SHARED.colorThreshold;
                return;
            }
            DIFFUSE += LIGHT_COLOR * SHADOW_CONTRIB;
        }
    \endcode

    \note SHARED can be written on all the functions without POST_PROCESS but it
    is safe to write it on MAIN and read on the other functions.

    \note A recommended use case to write SHARED on LIGHT functions is
    reseting it on MAIN first and then accumulating it on each LIGHT functions.

    \badcode
        SHARED_VARS {
            float sheenIntensity;
            float sheenRoughness;
            vec3 sheenColor;
            vec3 outSheenColor;
        };
        void MAIN()
        {
            ...
            vec4 tex = texture(uSheenMap, UV0);
            SHARED.sheenColor = tex.rgb;
            SHARED.sheenIntensity = tex.a;
            SHARED.sheenRoughness = uSheenRoughness;
            SHARED.outSheenColor = vec3(0.0);
        }
        void SPECULAR_LIGHT()
        {
            SHARED.outSheenColor += ...;
        }
        void POST_PROCESS()
        {
            COLOR_SUM = DIFFUSE + SPECULAR + EMISSIVE + SHARED.outSheenColor;
        }
    \endcode

    \note MAIN is called before others, and POST_PROCESS after all others,
    but that there is no guarantee for any other ordering for light processors.

    \section2 Additional special keywords

    The custom fragment shader code can freely access uniforms (such as, \c
    CAMERA_DIRECTION or \c CAMERA_POSITION), and varyings passed on from the
    custom vertex shader. Additionally, there are a number of built-in varyings
    available as special keywords. Some of these are optional in the sense that
    a vertex \c MAIN could calculate and pass on these on its own, but to
    reduce duplicated data fragment shaders can also rely on these built-ins
    instead. These built-ins are available in light processor functions and in
    the fragment MAIN.

    \list

    \li vec3 \c VAR_WORLD_NORMAL - Interpolated normal transformed by \c
    NORMAL_MATRIX.

    \li vec3 \c VAR_WORLD_TANGENT - Interpolated tangent transformed by \c
    MODEL_MATRIX.

    \li vec3 \c VAR_WORLD_BINORMAL - Interpolated binormal transformed by \c
    MODEL_MATRIX

    \li vec3 \c NORMAL - Unlike \c VAR_WORLD_NORMAL, which is the
    interpolated normal as-is, this value is potentially adjusted for
    double-sidedness: when rendering with culling disabled, the normal will get
    inverted as necessary. Therefore lighting and other calculations are
    recommended to use \c NORMAL instead of \c VAR_WORLD_NORMAL in order
    behave correctly with all culling modes.

    \li vec3 \c TANGENT - Like \c NORMAL, this value is potentially adjusted for
    double-sidedness: when rendering with culling disabled, the tangent will get
    inverted as necessary.

    \li vec3 \c BINORMAL - Like \c NORMAL, this value is potentially adjusted for
    double-sidedness: when rendering with culling disabled, the binormal will get
    inverted as necessary.

    \li vec3 \c VAR_WORLD_POSITION - Interpolated world space vertex position
    (\c{(MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz})

    \li vec4 \c VAR_COLOR - The interpolated vertex color when colors are
    provided in the mesh. \c{vec4(1.0)} otherwise.

    \li vec3 \c VIEW_VECTOR - Points towards the camera. This is
    effectively the \c{CAMERA_POSITION - VAR_WORLD_POSITION} vector normalized.

    \li vec4 \c FRAGCOORD - Contains the window-relative coordinates of the
    current fragment.

    \li float \c FRAMEBUFFER_Y_UP - The value is \c 1 when the Y axis points up
    in the coordinate system for framebuffers (textures), meaning \c{(0, 0)} is
    the bottom-left corner. The value is \c{-1} when the Y axis points down,
    \c{(0, 0)} being the top-left corner. Such differences in the underlying
    graphics APIs do not concern most custom materials. One notable exception
    is sampling \c SCREEN_TEXTURE with texture coordinates \b not based on
    \c FRAGCOORD. As the orientation of \c SCREEN_TEXTURE is tied to the
    underlying graphics API by nature, using texture coordinates from a mesh
    may need appropriate adjustments to the Y coordinate.

    For example, the following fragment shader, suitable for Rectangle or Cube
    meshes, will display the opaque objects from the scene on the model:

    \badcode
        VARYING vec2 texcoord;
        void MAIN()
        {
            vec2 screencoord = texcoord;
            if (FRAMEBUFFER_Y_UP < 0.0) // effectively: if not OpenGL
                screencoord.y = 1.0 - screencoord.y;
            BASE_COLOR = texture(SCREEN_TEXTURE, screencoord);
        }
    \endcode

    When sampling textures other than \c SCREEN_TEXTURE, and \c DEPTH_TEXTURE,
    or when \c FRAGCOORD is used to calculate the texture coordinate (which
    would be the typical use case for accessing the screen and depth textures),
    such an adjustment is not necessary.

    \li float \c NDC_Y_UP - The value is \c 1 when the Y axis points up in
    normalized device coordinate space, and \c{-1} when the Y axis points down.
    Y pointing down is the case when rendering happens with Vulkan. Most
    materials do not need to be concerned by this, but being able to branch
    based on this can become useful in certain advanced use cases.

    \li float \c NEAR_CLIP_VALUE - The value is \c -1 for when the clipping plane
    range's starts at \c -1 and goes to \c 1.  This is true when using OpenGL for
    rendering. For other rendering backends the value of this property will be
    \c 0 meaning the clipping plane range is \c 0 to \c 1. This value is useful
    with certain techniques involving the \c DEPTH_TEXTURE

    For example, the following fragment shader demonstrates a technique for
    reconstructing the position of a value from the depth buffer to determine
    the distance from the current position being rendered. When used in
    combination with \c INVERSE_PROJECTION_MATRIX the value of depth needs
    to be in normalized device coordinates so it is important to make sure that
    the range of depth value reflects that.  When the \c NEAR_CLIP_VALUE is
    \c -1 then the depth value gets scaled to be between \c -1 and \c 1.

    \badcode
        void MAIN() {
            vec2 screen_uv = FRAGCOORD.xy / vec2(textureSize(SCREEN_TEXTURE, 0));
            float depth = texture(DEPTH_TEXTURE, screen_uv).r;

            if (NEAR_CLIP_VALUE < 0.0) // effectively: if opengl
                depth = depth * 2.0 - 1.0;

            vec4 unproject = INVERSE_PROJECTION_MATRIX * vec4(screen_uv, depth, 1.0);
            depth = (unproject.xyz / unproject.w).z;
            float viewVectorZ = (VIEW_MATRIX * vec4(VAR_WORLD_POSITION, 1.0)).z;
            depth = viewVectorZ - depth;

            BASE_COLOR = vec4(depth, depth, depth, 1.0);
        }
    \endcode

    \li float \c IBL_EXPOSE - The amount of light emitted by the light probe.
    It comes from \l {SceneEnvironment::probeExposure}.
    \badcode
        DIFFUSE += AO_FACTOR * IBL_EXPOSE * BASE_COLOR.rgb * textureLod(IBL_TEXTURE, NORMAL, IBL_MAXMIPMAP).rgb;
    \endcode

    \li float \c IBL_HORIZON - The horizontal cut-off value of reflections from
    the lower half environment. It comes from \l {SceneEnvironment::probeHorizon}
    {Horizon Cut-Off} but remapped to [-1, 0).
    \badcode
        vec3 diffuse += AO_FACTOR * IBL_EXPOSE * BASE_COLOR.rgb * textureLod(IBL_TEXTURE, NORMAL, IBL_MAXMIPMAP).rgb;
        if (IBL_HORIZON > -1.0) {
            float ctr = 0.5 + 0.5 * IBL_HORIZON;
            float vertWt = smoothstep(ctr * 0.25, ctr + 0.25, NORMAL.y);
            float wtScaled = mix(1.0, vertWt, IBL_HORIZON + 1.0);
            diffuse *= wtScaled;
        }
    \endcode

    \li float \c IBL_MAXMIPMAP - The maximum mipmap level of IBL_TEXTURE.

    \endlist

    \section2 Instancing

    When doing instanced rendering, some of the keywords above do not apply.
    The following keywords are only available with instancing:

    \list
    \li \c INSTANCE_MODEL_MATRIX -> mat4, replacement for \c MODEL_MATRIX, including the instancing transformation.
    \li \c INSTANCE_MODELVIEWPROJECTION_MATRIX -> mat4, replacement for \c MODELVIEWPROJECTION_MATRIX, including the instancing transformation.
    \li \c INSTANCE_COLOR -> vec4, the instance color: to be combined with \c {COLOR}.
    \li \c INSTANCE_DATA -> vec4, instance custom data.
    \li \c INSTANCE_INDEX -> int, the instance number, and index into the instancing table.
    \endlist

    \section1 Screen, depth, and other textures

    The rendering pipeline can expose a number of textures to the custom
    material shaders with content from special render passes. This applies both
    to shaded and unshaded custom materials.

    For example, a shader may want access to a depth texture that contains the
    depth buffer contents for the opaque objects in the scene. This is achieved
    by sampling \c DEPTH_TEXTURE. Such a texture is not normally generated,
    unless there is a real need for it. Therefore, the presence of the
    following keywords in the vertex or fragment shader also acts as a toggle
    for opting in to the - potentially expensive - passes for generating the
    texture in question. (of course, it could be that some of these become
    already enabled due to other settings, such as the ambient occlusion
    parameters in SceneEnvironment or due to a post-processing effect relying
    on the depth texture, in which case the textures in question are generated
    regardless of the custom material and so sampling these special textures in
    the material comes at no extra cost apart from the texture access itself)

    \list

    \li \c SCREEN_TEXTURE - When present, a texture (\c sampler2D or \c
    sampler2DArray) with the color buffer from a rendering pass containing the
    contents of the scene excluding any transparent materials or any materials
    also using the SCREEN_TEXTURE is exposed to the shader under this name. The
    texture can be used for techniques that require the contents of the
    framebuffer they are being rendered to. The SCREEN_TEXTURE texture uses the
    same clear mode as the View3D. The size of these textures matches the size
    of the View3D in pixels. For example, a fragment shader could contain the
    following:
    \badcode
        vec2 uv = FRAGCOORD.xy / vec2(textureSize(SCREEN_TEXTURE, 0));
        vec2 displace = vec2(0.1);
        vec4 c = texture(SCREEN_TEXTURE, uv + displace);
    \endcode

    Be aware that using \c SCREEN_TEXTURE requires appropriate, conscious
    design of the scene. Objects using such materials have to be positioned
    carefully, typically above all other objects that are expected to be
    visible in the texture. Objects that employ semi-transparency in some form
    are never part of the \c SCREEN_TEXTURE. Often \c SCREEN_TEXTURE will be
    used in combination with \c BASE_COLOR in \c MAIN. For example, the
    following custom fragment shader applies an emboss effect, while keeping
    fragments not touched by opaque objects transparent. This assumes that the
    object with the material is placed in the front, and that it has blending
    enabled. \badcode
        void MAIN()
        {
            vec2 size = vec2(textureSize(SCREEN_TEXTURE, 0));
            vec2 uv = FRAGCOORD.xy / size;

            // basic emboss effect
            vec2 d = vec2(1.0 / size.x, 1.0 / size.y);
            vec4 diff = texture(SCREEN_TEXTURE, uv + d) - texture(SCREEN_TEXTURE, uv - d);
            float c = (diff.x + diff.y + diff.z) + 0.5;

            float alpha = texture(SCREEN_TEXTURE, uv).a;
            BASE_COLOR = vec4(vec3(c), alpha);
        }
    \endcode
    With \l{Multiview Rendering}{multiview rendering}, \c SCREEN_TEXTURE is a \c
    sampler2DArray. Use \c VIEW_INDEX to select the layer to use. For VR/AR
    applications that wish to support both types of rendering, the portable
    approach is the following:
    \badcode
        #if QSHADER_VIEW_COUNT >= 2
            vec4 c = texture(SCREEN_TEXTURE, vec3(uv, VIEW_INDEX));
        #else
            vec4 c = texture(SCREEN_TEXTURE, uv);
        #endif
    \endcode

    \li \c SCREEN_MIP_TEXTURE - Identical to \c SCREEN_TEXTURE in most ways,
    the difference being that this texture has mipmaps generated. This can be
    an expensive feature performance-wise, depending on the screen size, and
    due to having to generate the mipmaps every time the scene is rendered.
    Therefore, prefer using \c SCREEN_TEXTURE always, unless a technique
    relying on the texture mip levels (e.g. using \c textureLod in the shader)
    is implemented by the custom material.

    \li \c DEPTH_TEXTURE - When present, a texture (\c sampler2D or \c
    sampler2DArray) with the (non-linearized) depth buffer contents is exposed
    to the shader under this name. Only opaque objects are included.
    For example, a fragment shader could contain the following: \badcode
        ivec2 dtSize = textureSize(DEPTH_TEXTURE, 0);
        vec2 dtUV = (FRAGCOORD.xy) / vec2(dtSize);
        vec4 depthSample = texture(DEPTH_TEXTURE, dtUV);
        float zNear = CAMERA_PROPERTIES.x;
        float zFar = CAMERA_PROPERTIES.y;
        float zRange = zFar - zNear;
        float z_n = 2.0 * depthSample.r - 1.0;
        float d = 2.0 * zNear * zFar / (zFar + zNear - z_n * zRange);
        d /= zFar;
    \endcode
    With \l{Multiview Rendering}{multiview rendering}, \c DEPTH_TEXTURE is a \c
    sampler2DArray. Use \c VIEW_INDEX to select the layer to use. For VR/AR
    applications that wish to support both types of rendering, the portable
    approach is the following:
    \badcode
        #if QSHADER_VIEW_COUNT >= 2
            vec4 depthSample = texture(DEPTH_TEXTURE, vec3(uv, VIEW_INDEX));
        #else
            vec4 depthSample = texture(DEPTH_TEXTURE, uv);
        #endif
    \endcode

    \li \c NORMAL_ROUGHNESS_TEXTURE - When present, a texture (\c sampler2D)
    with the world-space normals and the material roughness is exposed to the
    shader under this name. Only opaque objects are included. The roughness is
    stored in the alpha channel.
    For example, a fragment shader could contain the following: \badcode
        vec3 N = normalize(texture(NORMAL_ROUGHNESS_TEXTURE, uv).rgb);
    \endcode

    \li \c AO_TEXTURE - When present and screen space ambient occlusion is
    enabled (meaning when the AO strength and distance are both non-zero) in
    SceneEnvironment, the SSAO texture (\c sampler2D or \c sampler2DArray) is
    exposed to the shader under this name. Sampling this texture can be useful
    in unshaded materials. Shaded materials have ambient occlusion support built
    in. This means that the ambient occlusion factor is taken into account
    automatically. Whereas in a fragment shader for an unshaded material one
    could write the following to achieve the same: \badcode
        ivec2 aoSize = textureSize(AO_TEXTURE, 0);
        vec2 aoUV = (FRAGCOORD.xy) / vec2(aoSize);
        float aoFactor = texture(AO_TEXTURE, aoUV).x;
    \endcode
    With \l{Multiview Rendering}{multiview rendering}, \c AO_TEXTURE is a \c
    sampler2DArray. Use \c VIEW_INDEX to select the layer to use. For VR/AR
    applications that wish to support both types of rendering, the portable
    approach is the following:
    \badcode
        #if QSHADER_VIEW_COUNT >= 2
            ivec2 aoSize = textureSize(AO_TEXTURE, 0).xy;
            vec2 aoUV = (FRAGCOORD.xy) / vec2(aoSize);
            float aoFactor = texture(AO_TEXTURE, vec3(aoUV, VIEW_INDEX)).x;
        #else
            ivec2 aoSize = textureSize(AO_TEXTURE, 0);
            vec2 aoUV = (FRAGCOORD.xy) / vec2(aoSize);
            float aoFactor = texture(AO_TEXTURE, aoUV).x;
        #endif
    \endcode

    \li \c IBL_TEXTURE - It will not enable any special rendering pass, but it can
    be used when the material has \l {Material::lightProbe} or the model is in the scope of
    \l {SceneEnvironment::lightProbe}.

    \badcode
        void IBL_PROBE()
        {
            DIFFUSE += AO_FACTOR * BASE_COLOR.rgb * textureLod(IBL_TEXTURE, NORMAL, IBL_MAXMIPMAP).rgb;
        }
    \endcode

    \li \c MOTION_VECTOR_TEXTURE — Enables a dedicated rendering pass that computes per-object
    motion vectors for the scene.
    The output is a four-channel texture: the R and G components contain the scaled motion vectors
    of the models, while the B and A components store the unscaled motion vectors.

    \li \c VIEW_INDEX - When used in the custom shader code, this is a
    (non-interpolated) uint variable. When \l{Multiview Rendering}{multiview
    rendering} is not used, the value is always 0. With multiview rendering, the
    value is the current view index (e.g., gl_ViewIndex). Useful in particular
    in combination with \c DEPTH_TEXTURE and similar when multiview rendering is
    enabled.

    \endlist

    \sa {Qt Quick 3D - Custom Shaders Example}, {Qt Quick 3D - Custom Materials Example}, {Programmable Materials, Effects, Geometry, and Texture data}
*/

/*!
    \qmlproperty url CustomMaterial::vertexShader

    Specfies the file with the snippet of custom vertex shader code.

    The value is a URL and must either be a local file or use the qrc scheme to
    access files embedded via the Qt resource system. Relative file paths
    (without a scheme) are also accepted, in which case the file is treated as
    relative to the component (the \c{.qml} file).

    \warning Shader snippets are assumed to be trusted content. Application
    developers are advised to carefully consider the potential implications
    before allowing the loading of user-provided content that is not part of the
    application.

    \sa fragmentShader
*/

/*!
    \qmlproperty url CustomMaterial::fragmentShader

    Specfies the file with the snippet of custom fragment shader code.

    The value is a URL and must either be a local file or use the qrc scheme to
    access files embedded via the Qt resource system. Relative file paths
    (without a scheme) are also accepted, in which case the file is treated as
    relative to the component (the \c{.qml} file).

    \warning Shader snippets are assumed to be trusted content. Application
    developers are advised to carefully consider the potential implications
    before allowing the loading of user-provided content that is not part of the
    application.

    \sa vertexShader
*/

/*!
    \qmlproperty enumeration CustomMaterial::shadingMode
    Specifies the type of the material. The default value is Shaded.

    \value CustomMaterial.Unshaded
    \value CustomMaterial.Shaded
*/

/*!
    \qmlproperty bool CustomMaterial::alwaysDirty
    Specifies that the material state is always dirty, which indicates that the material needs
    to be refreshed every time it is used by the QtQuick3D.
*/

/*!
    \qmlproperty enumeration CustomMaterial::sourceBlend

    Specifies the source blend factor. The default value is \c
    CustomMaterial.NoBlend.

    \value CustomMaterial.NoBlend
    \value CustomMaterial.Zero
    \value CustomMaterial.One
    \value CustomMaterial.SrcColor
    \value CustomMaterial.OneMinusSrcColor
    \value CustomMaterial.DstColor
    \value CustomMaterial.OneMinusDstColor
    \value CustomMaterial.SrcAlpha
    \value CustomMaterial.OneMinusSrcAlpha
    \value CustomMaterial.DstAlpha
    \value CustomMaterial.OneMinusDstAlpha
    \value CustomMaterial.ConstantColor
    \value CustomMaterial.OneMinusConstantColor
    \value CustomMaterial.ConstantAlpha
    \value CustomMaterial.OneMinusConstantAlpha
    \value CustomMaterial.SrcAlphaSaturate

    \note Both \l sourceBlend and \l destinationBlend needs to be set to a non-default
    value before blending is enabled.

    \sa destinationBlend
*/

/*!
    \qmlproperty enumeration CustomMaterial::destinationBlend

    Specifies the destination blend factor. The default value is \c
    CustomMaterial.NoBlend.

    \value CustomMaterial.NoBlend
    \value CustomMaterial.Zero
    \value CustomMaterial.One
    \value CustomMaterial.SrcColor
    \value CustomMaterial.OneMinusSrcColor
    \value CustomMaterial.DstColor
    \value CustomMaterial.OneMinusDstColor
    \value CustomMaterial.SrcAlpha
    \value CustomMaterial.OneMinusSrcAlpha
    \value CustomMaterial.DstAlpha
    \value CustomMaterial.OneMinusDstAlpha
    \value CustomMaterial.ConstantColor
    \value CustomMaterial.OneMinusConstantColor
    \value CustomMaterial.ConstantAlpha
    \value CustomMaterial.OneMinusConstantAlpha
    \value CustomMaterial.SrcAlphaSaturate

    \note Both \l sourceBlend and \l destinationBlend needs to be set to a non-default
    value before blending is enabled.

    \sa sourceBlend
*/

/*!
    \qmlproperty real CustomMaterial::lineWidth

    This property determines the width of the lines rendered, when the geometry
    is using a primitive type of lines or line strips. The default value is
    1.0. This property is not relevant when rendering other types of geometry,
    such as, triangle meshes.

    \warning Line widths other than 1 may not be suported at run time,
    depending on the underlying graphics API. When that is the case, the
    request to change the width is ignored. For example, none of the following
    can be expected to support wide lines: Direct3D, Metal, OpenGL with core
    profile contexts.

    \note Unlike the line width, the value of which is part of the graphics
    pipeline object, the point size for geometries with a topology of points is
    controlled by the vertex shader (when supported), and has therefore no
    corresponding QML property.
*/

/*!
    \qmlproperty enumeration CustomMaterial::sourceAlphaBlend
    \since 6.7

    Specifies the source alpha blend factor. The default value is \c
    CustomMaterial.NoBlend. This value is only actively used if \l sourceBlend and
    \l destinationBlend is set to a non-default value.

    \value CustomMaterial.NoBlend
    \value CustomMaterial.Zero
    \value CustomMaterial.One
    \value CustomMaterial.SrcColor
    \value CustomMaterial.OneMinusSrcColor
    \value CustomMaterial.DstColor
    \value CustomMaterial.OneMinusDstColor
    \value CustomMaterial.SrcAlpha
    \value CustomMaterial.OneMinusSrcAlpha
    \value CustomMaterial.DstAlpha
    \value CustomMaterial.OneMinusDstAlpha
    \value CustomMaterial.ConstantColor
    \value CustomMaterial.OneMinusConstantColor
    \value CustomMaterial.ConstantAlpha
    \value CustomMaterial.OneMinusConstantAlpha
    \value CustomMaterial.SrcAlphaSaturate

    \note For backwards compatibility purposes, when left to its default value,
    will be assigned the same value as \l sourceBlend when \l sourceBlend and
    \l destinationBlend is set to non-default values.

    \sa sourceBlend
*/

/*!
    \qmlproperty enumeration CustomMaterial::destinationAlphaBlend
    \since 6.7

    Specifies the destination alpha blend factor. The default value is \c
    CustomMaterial.NoBlend. This value is only actively used if \l sourceBlend and
    \l destinationBlend is set to a non-default value.

    \value CustomMaterial.NoBlend
    \value CustomMaterial.Zero
    \value CustomMaterial.One
    \value CustomMaterial.SrcColor
    \value CustomMaterial.OneMinusSrcColor
    \value CustomMaterial.DstColor
    \value CustomMaterial.OneMinusDstColor
    \value CustomMaterial.SrcAlpha
    \value CustomMaterial.OneMinusSrcAlpha
    \value CustomMaterial.DstAlpha
    \value CustomMaterial.OneMinusDstAlpha
    \value CustomMaterial.ConstantColor
    \value CustomMaterial.OneMinusConstantColor
    \value CustomMaterial.ConstantAlpha
    \value CustomMaterial.OneMinusConstantAlpha
    \value CustomMaterial.SrcAlphaSaturate

    \note For backwards compatibility purposes, when left to its default value,
    will be assigned the same value as \l destinationBlend when \l sourceBlend and
    \l destinationBlend is set to non-default values.

    \sa destinationBlend
*/

static inline QRhiGraphicsPipeline::BlendFactor toRhiBlendFactor(QQuick3DCustomMaterial::BlendMode mode)
{
    switch (mode) {
    case QQuick3DCustomMaterial::BlendMode::Zero:
        return QRhiGraphicsPipeline::Zero;
    case QQuick3DCustomMaterial::BlendMode::One:
        return QRhiGraphicsPipeline::One;
    case QQuick3DCustomMaterial::BlendMode::SrcColor:
        return QRhiGraphicsPipeline::SrcColor;
    case QQuick3DCustomMaterial::BlendMode::OneMinusSrcColor:
        return QRhiGraphicsPipeline::OneMinusSrcColor;
    case QQuick3DCustomMaterial::BlendMode::DstColor:
        return QRhiGraphicsPipeline::DstColor;
    case QQuick3DCustomMaterial::BlendMode::OneMinusDstColor:
        return QRhiGraphicsPipeline::OneMinusDstColor;
    case QQuick3DCustomMaterial::BlendMode::SrcAlpha:
        return QRhiGraphicsPipeline::SrcAlpha;
    case QQuick3DCustomMaterial::BlendMode::OneMinusSrcAlpha:
        return QRhiGraphicsPipeline::OneMinusSrcAlpha;
    case QQuick3DCustomMaterial::BlendMode::DstAlpha:
        return QRhiGraphicsPipeline::DstAlpha;
    case QQuick3DCustomMaterial::BlendMode::OneMinusDstAlpha:
        return QRhiGraphicsPipeline::OneMinusDstAlpha;
    case QQuick3DCustomMaterial::BlendMode::ConstantColor:
        return QRhiGraphicsPipeline::ConstantColor;
    case QQuick3DCustomMaterial::BlendMode::OneMinusConstantColor:
        return QRhiGraphicsPipeline::OneMinusConstantColor;
    case QQuick3DCustomMaterial::BlendMode::ConstantAlpha:
        return QRhiGraphicsPipeline::ConstantAlpha;
    case QQuick3DCustomMaterial::BlendMode::OneMinusConstantAlpha:
        return QRhiGraphicsPipeline::OneMinusConstantAlpha;
    case QQuick3DCustomMaterial::BlendMode::SrcAlphaSaturate:
        return QRhiGraphicsPipeline::SrcAlphaSaturate;
    default:
        return QRhiGraphicsPipeline::One;
    }
}

QQuick3DCustomMaterial::QQuick3DCustomMaterial(QQuick3DObject *parent)
    : QQuick3DMaterial(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::CustomMaterial)), parent)
{
}

QQuick3DCustomMaterial::~QQuick3DCustomMaterial() {}

QQuick3DCustomMaterial::BlendMode QQuick3DCustomMaterial::srcBlend() const
{
    return m_srcBlend;
}

void QQuick3DCustomMaterial::setSrcBlend(BlendMode mode)
{
    if (m_srcBlend == mode)
        return;

    m_srcBlend = mode;
    update();
    emit srcBlendChanged();
}

QQuick3DCustomMaterial::BlendMode QQuick3DCustomMaterial::dstBlend() const
{
    return m_dstBlend;
}

void QQuick3DCustomMaterial::setDstBlend(BlendMode mode)
{
    if (m_dstBlend == mode)
        return;

    m_dstBlend = mode;
    update();
    emit dstBlendChanged();
}

QQuick3DCustomMaterial::BlendMode QQuick3DCustomMaterial::srcAlphaBlend() const
{
    return m_srcAlphaBlend;
}

void QQuick3DCustomMaterial::setSrcAlphaBlend(QQuick3DCustomMaterial::BlendMode mode)
{
    if (m_srcAlphaBlend == mode)
        return;

    m_srcAlphaBlend = mode;
    update();
    emit srcAlphaBlendChanged();
}

QQuick3DCustomMaterial::BlendMode QQuick3DCustomMaterial::dstAlphaBlend() const
{
    return m_dstAlphaBlend;
}

void QQuick3DCustomMaterial::setDstAlphaBlend(QQuick3DCustomMaterial::BlendMode mode)
{
    if (m_dstAlphaBlend == mode)
        return;

    m_dstAlphaBlend = mode;
    update();
    emit dstAlphaBlendChanged();
}

QQuick3DCustomMaterial::ShadingMode QQuick3DCustomMaterial::shadingMode() const
{
    return m_shadingMode;
}

void QQuick3DCustomMaterial::setShadingMode(ShadingMode mode)
{
    if (m_shadingMode == mode)
        return;

    m_shadingMode = mode;
    markDirty(*this, Dirty::ShaderSettingsDirty);
    emit shadingModeChanged();
}

QUrl QQuick3DCustomMaterial::vertexShader() const
{
    return m_vertexShader;
}

void QQuick3DCustomMaterial::setVertexShader(const QUrl &url)
{
    if (m_vertexShader == url)
        return;

    m_vertexShader = url;
    markDirty(*this, Dirty::ShaderSettingsDirty);
    emit vertexShaderChanged();
}

QUrl QQuick3DCustomMaterial::fragmentShader() const
{
    return m_fragmentShader;
}

void QQuick3DCustomMaterial::setFragmentShader(const QUrl &url)
{
    if (m_fragmentShader == url)
        return;

    m_fragmentShader = url;
    markDirty(*this, Dirty::ShaderSettingsDirty);
    emit fragmentShaderChanged();
}


QString QQuick3DCustomMaterial::vertexShaderCode() const
{
    return m_vertexShaderCode;
}

void QQuick3DCustomMaterial::setVertexShaderCode(const QString &code)
{
    if (m_vertexShaderCode == code)
        return;

    m_vertexShaderCode = code;
    markDirty(*this, Dirty::ShaderSettingsDirty);
    emit vertexShaderCodeChanged();
}

QString QQuick3DCustomMaterial::fragmentShaderCode() const
{
    return m_fragmentShaderCode;
}

void QQuick3DCustomMaterial::setFragmentShaderCode(const QString &code)
{
    if (m_fragmentShaderCode == code)
        return;

    m_fragmentShaderCode = code;
    markDirty(*this, Dirty::ShaderSettingsDirty);
    emit fragmentShaderCodeChanged();
}

float QQuick3DCustomMaterial::lineWidth() const
{
    return m_lineWidth;
}

void QQuick3DCustomMaterial::setLineWidth(float width)
{
    if (qFuzzyCompare(m_lineWidth, width))
        return;
    m_lineWidth = width;
    update();
    emit lineWidthChanged();
}

void QQuick3DCustomMaterial::markAllDirty()
{
    m_dirtyAttributes |= Dirty::AllDirty;
    QQuick3DMaterial::markAllDirty();
}

void QQuick3DCustomMaterial::markDirty(QQuick3DCustomMaterial &that, Dirty type)
{
    if (!(that.m_dirtyAttributes & quint32(type))) {
        that.m_dirtyAttributes |= quint32(type);
        that.update();
    }
}

bool QQuick3DCustomMaterial::alwaysDirty() const
{
    return m_alwaysDirty;
}

void QQuick3DCustomMaterial::setAlwaysDirty(bool alwaysDirty)
{
    if (m_alwaysDirty == alwaysDirty)
        return;

    m_alwaysDirty = alwaysDirty;
    update();
    emit alwaysDirtyChanged();
}

static void setCustomMaterialFlagsFromShader(QSSGRenderCustomMaterial *material, const QSSGCustomShaderMetaData &meta)
{
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesScreenTexture))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::ScreenTexture, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesScreenMipTexture))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::ScreenMipTexture, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesDepthTexture))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::DepthTexture, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesNormalTexture))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::NormalTexture, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesAoTexture))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::AoTexture, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesProjectionMatrix))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::ProjectionMatrix, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesInverseProjectionMatrix))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::InverseProjectionMatrix, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesVarColor))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::VarColor, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesIblOrientation))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::IblOrientation, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesLightmap))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::Lightmap, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesSkinning))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::Skinning, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesMorphing))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::Morphing, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesViewIndex))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::ViewIndex, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesClearcoat))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::Clearcoat, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesClearcoatFresnelScaleBias))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::ClearcoatFresnelScaleBias, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesFresnelScaleBias))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::FresnelScaleBias, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesMotionVectorTexture))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::MotionVectorTexture, true);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesTransmission)) {
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::Transmission, true);
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::ScreenTexture, true);
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::ScreenMipTexture, true);
    }

    // vertex only
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::OverridesPosition))
        material->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::OverridesPosition, true);

    // fragment only
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesSharedVars))
        material->m_usesSharedVariables = true;
}

static QByteArray prepareCustomShader(QSSGRenderCustomMaterial *customMaterial,
                                      const QSSGShaderCustomMaterialAdapter::StringPairList &uniforms,
                                      const QByteArray &snippet,
                                      QSSGShaderCache::ShaderType shaderType,
                                      QSSGCustomShaderMetaData &meta,
                                      bool multiViewCompatible)
{
    if (snippet.isEmpty())
        return QByteArray();

    QByteArray sourceCode = snippet;
    QByteArray buf;

    QSSGShaderCustomMaterialAdapter::ShaderCodeAndMetaData result;
    QSSGShaderCustomMaterialAdapter::CustomShaderPrepWorkData scratch;
    QSSGShaderCustomMaterialAdapter::beginPrepareCustomShader(&scratch, &result, sourceCode, shaderType, multiViewCompatible);
    QSSGShaderCustomMaterialAdapter::finishPrepareCustomShader(&buf, scratch, result, shaderType, multiViewCompatible, uniforms, {}, {}, {}, {});

    sourceCode = result.first;
    sourceCode.append(buf);
    meta = result.second;
    setCustomMaterialFlagsFromShader(customMaterial, meta);
    return sourceCode;
}

QSSGRenderGraphObject *QQuick3DCustomMaterial::updateSpatialNode(QSSGRenderGraphObject *node)
{
    using namespace QSSGShaderUtils;

    const auto &renderContext = QQuick3DObjectPrivate::get(this)->sceneManager->wattached->rci();
    if (!renderContext) {
        qWarning("QQuick3DCustomMaterial: No render context interface?");
        return nullptr;
    }

    QSSGShaderCustomMaterialAdapter::StringPairList uniforms;
    QSSGRenderCustomMaterial *customMaterial = static_cast<QSSGRenderCustomMaterial *>(node);
    bool newBackendNode = false;
    bool shadersMayChange = false;
    if (!customMaterial) {
        customMaterial = new QSSGRenderCustomMaterial;
        newBackendNode = true;
    } else if (m_dirtyAttributes & ShaderSettingsDirty) {
        shadersMayChange = true;
    }

    if (newBackendNode || shadersMayChange) {
        markAllDirty();

        customMaterial->m_properties.clear();
        customMaterial->m_textureProperties.clear();

        customMaterial->m_shadingMode = QSSGRenderCustomMaterial::ShadingMode(int(m_shadingMode));

        QMetaMethod propertyDirtyMethod;
        const int idx = metaObject()->indexOfSlot("onPropertyDirty()");
        if (idx != -1)
            propertyDirtyMethod = metaObject()->method(idx);

        const int propCount = metaObject()->propertyCount();
        int propOffset = metaObject()->propertyOffset();

        // Custom materials can have multilayered inheritance structure, so find the actual propOffset
        const QMetaObject *superClass = metaObject()->superClass();
        while (superClass && qstrcmp(superClass->className(), "QQuick3DCustomMaterial") != 0)  {
            propOffset = superClass->propertyOffset();
            superClass = superClass->superClass();
        }

        using TextureInputProperty = QPair<QQuick3DShaderUtilsTextureInput *, const char *>;
        QVector<TextureInputProperty> textureProperties; // We'll deal with these later

        for (int i = propOffset; i != propCount; ++i) {
            const auto property = metaObject()->property(i);
            if (Q_UNLIKELY(!property.isValid()))
                continue;

            const auto name = property.name();
            QMetaType propType = property.metaType();
            QVariant propValue = property.read(this);
            if (propType == QMetaType(QMetaType::QVariant))
                propType = propValue.metaType();

            if (propType.id() >= QMetaType::User) {
                if (propType.id() == qMetaTypeId<QQuick3DShaderUtilsTextureInput *>()) {
                    if (QQuick3DShaderUtilsTextureInput *texture = property.read(this).value<QQuick3DShaderUtilsTextureInput *>())
                        textureProperties.push_back({texture, name});
                }
            } else if (propType == QMetaType(QMetaType::QObjectStar)) {
                if (QQuick3DShaderUtilsTextureInput *texture = qobject_cast<QQuick3DShaderUtilsTextureInput *>(propValue.value<QObject *>()))
                    textureProperties.push_back({texture, name});
            } else {
                const auto type = uniformType(propType);
                if (type != QSSGRenderShaderValue::Unknown) {
                    uniforms.append({ uniformTypeName(propType), name });
                    customMaterial->m_properties.push_back({ name, propValue, uniformType(propType), i});
                    if (newBackendNode) {
                        // Track the property changes
                        if (property.hasNotifySignal() && propertyDirtyMethod.isValid())
                            connect(this, property.notifySignal(), this, propertyDirtyMethod);
                    } // else already connected
                } else {
                    // ### figure out how _not_ to warn when there are no dynamic
                    // properties defined (because warnings like Blah blah objectName etc. are not helpful)
                    //qWarning("No known uniform conversion found for effect property %s. Skipping", property.name());
                }
            }
        }

        const auto processTextureProperty = [&](QQuick3DShaderUtilsTextureInput &texture, const QByteArray &name) {
            texture.name = name;

            QSSGRenderCustomMaterial::TextureProperty textureData;
            textureData.texInput = &texture;
            textureData.name = name;
            textureData.shaderDataType = QSSGRenderShaderValue::Texture;

            if (newBackendNode) {
                connect(&texture, &QQuick3DShaderUtilsTextureInput::enabledChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
                connect(&texture, &QQuick3DShaderUtilsTextureInput::textureChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
            } // else already connected

            QQuick3DTexture *tex = texture.texture(); // may be null if the TextureInput has no 'texture' set
            if (tex && QQuick3DObjectPrivate::get(tex)->type == QQuick3DObjectPrivate::Type::ImageCube) {
                uniforms.append({ QByteArrayLiteral("samplerCube"), textureData.name });
            } else if (tex && tex->textureData() && tex->textureData()->depth() > 0) {
                uniforms.append({ QByteArrayLiteral("sampler3D"), textureData.name });
            } else if (tex && tex->textureProvider() && QQuick3DObjectPrivate::get(tex->textureProvider())->type == QQuick3DObjectPrivate::Type::TextureProvider) {
                auto textureProvider = static_cast<QQuick3DTextureProviderExtension *>(tex->textureProvider());
                switch (textureProvider->samplerHint()) {
                case QQuick3DTextureProviderExtension::SamplerHint::Sampler2D:
                    uniforms.append({ QByteArrayLiteral("sampler2D"), textureData.name });
                    break;
                case QQuick3DTextureProviderExtension::SamplerHint::Sampler2DArray:
                    uniforms.append({ QByteArrayLiteral("sampler2DArray"), textureData.name });
                    break;
                case QQuick3DTextureProviderExtension::SamplerHint::Sampler3D:
                    uniforms.append({ QByteArrayLiteral("sampler3D"), textureData.name });
                    break;
                case QQuick3DTextureProviderExtension::SamplerHint::SamplerCube:
                    uniforms.append({ QByteArrayLiteral("samplerCube"), textureData.name });
                    break;
                case QQuick3DTextureProviderExtension::SamplerHint::SamplerCubeArray:
                    uniforms.append({ QByteArrayLiteral("samplerCubeArray"), textureData.name });
                    break;
                case QQuick3DTextureProviderExtension::SamplerHint::SamplerBuffer:
                    uniforms.append({ QByteArrayLiteral("samplerBuffer"), textureData.name });
                    break;
                }
            } else {
                uniforms.append({ QByteArrayLiteral("sampler2D"), textureData.name });
            }

            customMaterial->m_textureProperties.push_back(textureData);
        };

        for (const auto &textureProperty : std::as_const(textureProperties))
            processTextureProperty(*textureProperty.first, textureProperty.second);

        if (customMaterial->incompleteBuildTimeObject || (m_dirtyAttributes & DynamicPropertiesDirty)) { // This object came from the shadergen tool
            const auto names = dynamicPropertyNames();
            for (const auto &name : names) {
                QVariant propValue = property(name.constData());
                QMetaType propType = propValue.metaType();
                if (propType == QMetaType(QMetaType::QVariant))
                    propType = propValue.metaType();

                if (propType.id() >= QMetaType::User) {
                    if (propType.id() == qMetaTypeId<QQuick3DShaderUtilsTextureInput *>()) {
                        if (QQuick3DShaderUtilsTextureInput *texture = propValue.value<QQuick3DShaderUtilsTextureInput *>())
                            textureProperties.push_back({texture, name});
                    }
                } else if (propType.id() == QMetaType::QObjectStar) {
                    if (QQuick3DShaderUtilsTextureInput *texture = qobject_cast<QQuick3DShaderUtilsTextureInput *>(propValue.value<QObject *>()))
                        textureProperties.push_back({texture, name});
                } else {
                    const auto type = uniformType(propType);
                    if (type != QSSGRenderShaderValue::Unknown) {
                        uniforms.append({ uniformTypeName(propType), name });
                        customMaterial->m_properties.push_back({ name, propValue,
                                                                 uniformType(propType), -1 /* aka. dynamic property */});
                        // We don't need to track property changes
                    } else {
                        // ### figure out how _not_ to warn when there are no dynamic
                        // properties defined (because warnings like Blah blah objectName etc. are not helpful)
                        qWarning("No known uniform conversion found for custom material property %s. Skipping", name.constData());
                    }
                }
            }

            for (const auto &property : std::as_const(textureProperties))
                processTextureProperty(*property.first, property.second);
        }

        const QQmlContext *context = qmlContext(this);
        QByteArray vertex;
        QByteArray fragment;
        QByteArray vertexProcessed[2];
        QSSGCustomShaderMetaData vertexMeta;
        QByteArray fragmentProcessed[2];
        QSSGCustomShaderMetaData fragmentMeta;
        QByteArray shaderPathKey("custom material --");

        customMaterial->m_renderFlags = {};

        if (!m_vertexShader.isEmpty())
            vertex = QSSGShaderUtils::resolveShader(m_vertexShader, context, shaderPathKey);
        else if (!m_vertexShaderCode.isEmpty())
            vertex = m_vertexShaderCode.toLatin1();

        if (!m_fragmentShader.isEmpty())
            fragment = QSSGShaderUtils::resolveShader(m_fragmentShader, context, shaderPathKey);
        else if (!m_fragmentShaderCode.isEmpty())
            fragment = m_fragmentShaderCode.toLatin1();

        // Multiview is a problem, because we will get a dedicated snippet after
        // preparation (the one that has [qt_viewIndex] added where it matters).
        // But at least the view count plays no role here on this level. So one
        // normal and one multiview "variant" is good enough.

        vertexProcessed[QSSGRenderCustomMaterial::RegularShaderPathKeyIndex] =
            prepareCustomShader(customMaterial, uniforms, vertex, QSSGShaderCache::ShaderType::Vertex, vertexMeta, false);
        fragmentProcessed[QSSGRenderCustomMaterial::RegularShaderPathKeyIndex] =
            prepareCustomShader(customMaterial, uniforms, fragment, QSSGShaderCache::ShaderType::Fragment, fragmentMeta, false);

        vertexProcessed[QSSGRenderCustomMaterial::MultiViewShaderPathKeyIndex] =
            prepareCustomShader(customMaterial, uniforms, vertex, QSSGShaderCache::ShaderType::Vertex, vertexMeta, true);
        fragmentProcessed[QSSGRenderCustomMaterial::MultiViewShaderPathKeyIndex] =
            prepareCustomShader(customMaterial, uniforms, fragment, QSSGShaderCache::ShaderType::Fragment, fragmentMeta, true);

        // At this point we have snippets that look like this:
        //   - the original code, with VARYING ... lines removed
        //   - followed by QQ3D_SHADER_META block for uniforms
        //   - followed by QQ3D_SHADER_META block for inputs/outputs

        customMaterial->m_customShaderPresence = {};
        for (int i : { QSSGRenderCustomMaterial::RegularShaderPathKeyIndex, QSSGRenderCustomMaterial::MultiViewShaderPathKeyIndex }) {
            if (vertexProcessed[i].isEmpty() && fragmentProcessed[i].isEmpty())
                continue;

            const QByteArray key = shaderPathKey + ':' + QCryptographicHash::hash(QByteArray(vertexProcessed[i] + fragmentProcessed[i]), QCryptographicHash::Algorithm::Sha1).toHex();
            // the processed snippet code is different for regular and multiview, so 'key' reflects that already
            customMaterial->m_shaderPathKey[i] = key;
            if (!vertexProcessed[i].isEmpty()) {
                customMaterial->m_customShaderPresence.setFlag(QSSGRenderCustomMaterial::CustomShaderPresenceFlag::Vertex);
                renderContext->shaderLibraryManager()->setShaderSource(key, QSSGShaderCache::ShaderType::Vertex, vertexProcessed[i], vertexMeta);
            }
            if (!fragmentProcessed[i].isEmpty()) {
                customMaterial->m_customShaderPresence.setFlag(QSSGRenderCustomMaterial::CustomShaderPresenceFlag::Fragment);
                renderContext->shaderLibraryManager()->setShaderSource(key, QSSGShaderCache::ShaderType::Fragment, fragmentProcessed[i], fragmentMeta);
            }
        }
    }

    customMaterial->setAlwaysDirty(m_alwaysDirty);
    if (m_srcBlend != BlendMode::NoBlend && m_dstBlend != BlendMode::NoBlend) { // both must be set to something other than NoBlend
        customMaterial->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::Blending, true);
        customMaterial->m_srcBlend = toRhiBlendFactor(m_srcBlend);
        customMaterial->m_dstBlend = toRhiBlendFactor(m_dstBlend);
        // alpha blending is only active if rgb blending is
        if (m_srcAlphaBlend != BlendMode::NoBlend && m_dstAlphaBlend != BlendMode::NoBlend) {
            customMaterial->m_srcAlphaBlend = toRhiBlendFactor(m_srcAlphaBlend);
            customMaterial->m_dstAlphaBlend = toRhiBlendFactor(m_dstAlphaBlend);
        } else {
            customMaterial->m_srcAlphaBlend = customMaterial->m_srcBlend;
            customMaterial->m_dstAlphaBlend = customMaterial->m_dstBlend;
        }
    } else {
        customMaterial->m_renderFlags.setFlag(QSSGRenderCustomMaterial::RenderFlag::Blending, false);
    }
    customMaterial->m_lineWidth = m_lineWidth;

    QQuick3DMaterial::updateSpatialNode(customMaterial);

    if (m_dirtyAttributes & Dirty::PropertyDirty) {
        for (auto &prop : customMaterial->m_properties) {
            auto p = metaObject()->property(prop.pid);
            if (Q_LIKELY(p.isValid()))
                prop.value = p.read(this);
        }
    }

    if (m_dirtyAttributes & Dirty::TextureDirty) {
        for (QSSGRenderCustomMaterial::TextureProperty &prop : customMaterial->m_textureProperties) {
            QQuick3DTexture *tex = prop.texInput->texture();
            if (tex) {
                if (prop.texInput->enabled)
                    prop.texImage = tex->getRenderImage();
                else
                    prop.texImage = nullptr;
                prop.minFilterType = tex->minFilter() == QQuick3DTexture::Nearest ? QSSGRenderTextureFilterOp::Nearest
                                                                                  : QSSGRenderTextureFilterOp::Linear;
                prop.magFilterType = tex->magFilter() == QQuick3DTexture::Nearest ? QSSGRenderTextureFilterOp::Nearest
                                                                                  : QSSGRenderTextureFilterOp::Linear;
                prop.mipFilterType = tex->generateMipmaps() ? (tex->mipFilter() == QQuick3DTexture::Nearest ? QSSGRenderTextureFilterOp::Nearest
                                                                                                            : QSSGRenderTextureFilterOp::Linear)
                                                            : QSSGRenderTextureFilterOp::None;
                prop.horizontalClampType = tex->horizontalTiling() == QQuick3DTexture::Repeat ? QSSGRenderTextureCoordOp::Repeat
                                                            : (tex->horizontalTiling() == QQuick3DTexture::ClampToEdge) ? QSSGRenderTextureCoordOp::ClampToEdge
                                                            : QSSGRenderTextureCoordOp::MirroredRepeat;
                prop.verticalClampType = tex->verticalTiling() == QQuick3DTexture::Repeat ? QSSGRenderTextureCoordOp::Repeat
                                                            : (tex->verticalTiling() == QQuick3DTexture::ClampToEdge) ? QSSGRenderTextureCoordOp::ClampToEdge
                                                            : QSSGRenderTextureCoordOp::MirroredRepeat;
                prop.zClampType = tex->depthTiling() == QQuick3DTexture::Repeat ? QSSGRenderTextureCoordOp::Repeat
                        : (tex->depthTiling() == QQuick3DTexture::ClampToEdge)  ? QSSGRenderTextureCoordOp::ClampToEdge
                                                                            : QSSGRenderTextureCoordOp::MirroredRepeat;
            } else {
                prop.texImage = nullptr;
            }

            if (tex != prop.lastConnectedTexture) {
                prop.lastConnectedTexture = tex;
                disconnect(prop.minFilterChangedConn);
                disconnect(prop.magFilterChangedConn);
                disconnect(prop.mipFilterChangedConn);
                disconnect(prop.horizontalTilingChangedConn);
                disconnect(prop.verticalTilingChangedConn);
                disconnect(prop.depthTilingChangedConn);
                if (tex) {
                    prop.minFilterChangedConn = connect(tex, &QQuick3DTexture::minFilterChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
                    prop.magFilterChangedConn = connect(tex, &QQuick3DTexture::magFilterChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
                    prop.mipFilterChangedConn = connect(tex, &QQuick3DTexture::mipFilterChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
                    prop.horizontalTilingChangedConn = connect(tex, &QQuick3DTexture::horizontalTilingChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
                    prop.verticalTilingChangedConn = connect(tex, &QQuick3DTexture::verticalTilingChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
                    prop.depthTilingChangedConn = connect(tex, &QQuick3DTexture::depthTilingChanged, this, &QQuick3DCustomMaterial::onTextureDirty);
                }
            }
        }
    }

    m_dirtyAttributes = 0;

    return customMaterial;
}

void QQuick3DCustomMaterial::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    QQuick3DMaterial::itemChange(change, value);
    if (change == QQuick3DObject::ItemSceneChange) {
        if (auto sceneManager = value.sceneManager) {
            for (const auto &it : std::as_const(m_dynamicTextureMaps)) {
                if (auto tex = it->texture())
                    QQuick3DObjectPrivate::refSceneManager(tex, *sceneManager);
            }
        } else {
            for (const auto &it : std::as_const(m_dynamicTextureMaps)) {
                if (auto tex = it->texture())
                    QQuick3DObjectPrivate::derefSceneManager(tex);
            }
        }
    }
}

void QQuick3DCustomMaterial::onPropertyDirty()
{
    markDirty(*this, Dirty::PropertyDirty);
    update();
}

void QQuick3DCustomMaterial::onTextureDirty()
{
    markDirty(*this, Dirty::TextureDirty);
    update();
}

void QQuick3DCustomMaterial::setDynamicTextureMap(QQuick3DShaderUtilsTextureInput *textureMap)
{
    // There can only be one texture input per property, as the texture input is a combination
    // of the texture used and the uniform name!
    auto it = m_dynamicTextureMaps.constFind(textureMap);

    if (it == m_dynamicTextureMaps.constEnd()) {
        // Track the object, if it's destroyed we need to remove it from our table.
        connect(textureMap, &QQuick3DShaderUtilsTextureInput::destroyed, this, [this, textureMap]() {
            auto it = m_dynamicTextureMaps.constFind(textureMap);
            if (it != m_dynamicTextureMaps.constEnd())
                m_dynamicTextureMaps.erase(it);
        });
        m_dynamicTextureMaps.insert(textureMap);

        update();
    }
}

QT_END_NAMESPACE

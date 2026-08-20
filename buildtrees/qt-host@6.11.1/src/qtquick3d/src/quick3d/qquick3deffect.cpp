// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3deffect_p.h"

#include <ssg/qssgrendercontextcore.h>
#include <QtQuick3DRuntimeRender/private/qssgrendereffect_p.h>
#include <QtQuick3DRuntimeRender/private/qssgshadermaterialadapter_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3D/private/qquick3dscenemanager_p.h>
#include <QtCore/qfile.h>
#include <QtCore/qurl.h>


QT_BEGIN_NAMESPACE

/*!
    \qmltype Effect
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \nativetype QQuick3DEffect
    \brief Base component for creating a post-processing effect.

    The Effect type allows the user to implement their own post-processing
    effects for QtQuick3D.

    \section1 Post-processing effects

    A post-processing effect is conceptually very similar to Qt Quick's \l
    ShaderEffect item. When an effect is present, the scene is rendered into a
    separate texture first. The effect is then applied by drawing a textured
    quad to the main render target, depending on the
    \l{View3D::renderMode}{render mode} of the View3D. The effect can provide a
    vertex shader, a fragment shader, or both. Effects are always applied on the
    entire scene, per View3D.

    Effects are associated with the \l SceneEnvironment in the
    \l{SceneEnvironment::effects} property. The property is a list: effects can
    be chained together; they are applied in the order they are in the list,
    using the previous step's output as the input to the next one, with the last
    effect's output defining the contents of the View3D.

    \note \l SceneEnvironment and \l ExtendedSceneEnvironment provide a set of
    built-in effects, such as depth of field, glow/bloom, lens flare, color
    grading, and vignette. Always consider first if these are sufficient for
    the application's needs, and prefer using the built-in facilities instead
    of implementing a custom post-processing effect.

    Effects are similar to \l{CustomMaterial}{custom materials} in many
    ways. However, a custom material is associated with a model and is
    responsible for the shading of that given mesh. Whereas an effect's vertex
    shader always gets a quad (for example, two triangles) as its input, while
    its fragment shader samples the texture with the scene's content.

    Unlike custom materials, effects support multiple passes. For many effects
    this it not necessary, and when there is a need to apply multiple effects,
    identical results can often be achieved by chaining together multiple
    effects in \l{SceneEnvironment::effects}{the SceneEnvironment}. This is
    demonstrated by the \l{Qt Quick 3D - Custom Effect Example}{Custom Effect
    example} as well. However, passes have the possibility to request additional
    color buffers (texture), and specify which of these additional buffers they
    output to. This allows implementing more complex image processing techniques
    since subsequent passes can then use one or more of these additional
    buffers, plus the original scene's content, as their input. If necessary,
    these additional buffers can have an extended lifetime, meaning their
    content is preserved between frames, which allows implementing effects that
    rely on accumulating content from multiple frames, such as, motion blur.

    When compared to Qt Quick's 2D ShaderEffect, the 3D post-processing effects
    have the advantage of being able to work with depth buffer data, as well as
    the ability to implement multiple passes with intermediate buffers. In
    addition, the texture-related capabilities are extended: Qt Quick 3D allows
    more fine-grained control over filtering modes, and allows effects to work
    with texture formats other than RGBA8, for example, floating point formats.

    \note Post-processing effects are currently available when the View3D
    has its \l{View3D::renderMode}{renderMode} set to \c Offscreen,
    \c Underlay or \c Overlay. Effects will not be rendered for \c Inline mode.

    \note When using post-processing effects, the application-provided shaders
    should expect linear color data without tonemapping applied. The
    tonemapping that is performed during the main render pass (or during skybox
    rendering, if there is a skybox) when
    \l{SceneEnvironment::tonemapMode}{tonemapMode} is set to a value other than
    \c SceneEnvironment.TonemapModeNone, is automatically disabled when there
    is at least one post-processing effect specified in the SceneEnvironment.
    The last effect in the chain (more precisely, the last pass of the last
    effect in the chain) will automatically get its fragment shader amended to
    perform the same tonemapping the main render pass would.

    \note Effects that perform their own tonemapping should be used in a
    SceneEnvironment that has the built-in tonemapping disabled by setting
    \l{SceneEnvironment::tonemapMode}{tonemapMode} to \c
    SceneEnvironment.TonemapModeNone.

    \note By default the texture used as the effects' input is created with a
    floating point texture format, such as 16-bit floating point RGBA. The
    output texture's format is the same since by default it follows the input
    format. This can be overridden using \l Buffer and an empty name. The
    default RGBA16F is useful because it allows working with non-tonemapped
    linear data without having the color values outside the 0-1 range clamped.

    \section1 Exposing data to the shaders

    Like with CustomMaterial or ShaderEffect, the dynamic properties of an
    Effect object can be changed and animated using the usual QML and Qt Quick
    facilities, and the values are exposed to the shaders automatically. The
    following list shows how properties are mapped:

    \list
    \li bool, int, real -> bool, int, float
    \li QColor, \l{QtQml::Qt::rgba()}{color} -> vec4, and the color gets
    converted to linear, assuming sRGB space for the color value specified in
    QML. The built-in Qt colors, such as \c{"green"} are in sRGB color space as
    well, and the same conversion is performed for all color properties of
    DefaultMaterial and PrincipledMaterial, so this behavior of Effect
    matches those.
    \li QRect, QRectF, \l{QtQml::Qt::rect()}{rect} -> vec4
    \li QPoint, QPointF, \l{QtQml::Qt::point()}{point}, QSize, QSizeF, \l{QtQml::Qt::size()}{size} -> vec2
    \li QVector2D, \l{QtQml::Qt::vector2d()}{vector2d} -> vec3
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
    effects, as the rest (such as, UV transformations) is up to the custom
    shaders to implement as they see fit.

    \endlist

    \note When a uniform referenced in the shader code does not have a
    corresponding property, it will cause a shader compilation error when
    processing the effect at run time. There are some exceptions to this,
    such as, sampler uniforms, that get a dummy texture bound when no
    corresponding QML property is present, but as a general rule, all uniforms
    and samplers must have a corresponding property declared in the
    Effect object.

    \section1 Getting started with user-defined effects

    A custom post-processing effect involves at minimum an Effect object and a
    fragment shader snippet. Some effects will also want a customized vertex
    shader as well.

    As a simple example, let's create an effect that combines the scene's
    content with an image, while further altering the red channel's value in an
    animated manner:

    \table 70%
    \row
    \li \qml
    Effect {
        id: simpleEffect
        property TextureInput tex: TextureInput {
            texture: Texture { source: "image.png" }
        }
        property real redLevel
        NumberAnimation on redLevel { from: 0; to: 1; duration: 5000; loops: -1 }
        passes: Pass {
           shaders: Shader {
               stage: Shader.Fragment
               shader: "effect.frag"
           }
        }
    }
    \endqml
    \li \badcode
    void MAIN()
    {
        vec4 c = texture(tex, TEXTURE_UV);
        c.r *= redLevel;
        FRAGCOLOR = c * texture(INPUT, INPUT_UV);
    }
    \endcode
    \endtable

    Here the texture with the image \c{image.png} is exposed to the shader under
    the name \c tex. The value of redLevel is available in the shader in a \c
    float uniform with the same name.

    The fragment shader must contain a function called \c MAIN. The final
    fragment color is determined by \c FRAGCOLOR. The main input texture, with
    the contents of the View3D's scene, is accessible under a \c sampler2D with
    the name \c INPUT. The UV coordinates from the quad are in \c
    INPUT_UV. These UV values are always suitable for sampling \c INPUT,
    regardless of the underlying graphics API at run time (and so regardless of
    the Y axis direction in images since the necessary adjustments are applied
    automatically by Qt Quick 3D). Sampling the texture with our external image
    is done using \c TEXTURE_UV. \c INPUT_UV is not suitable in cross-platform
    applications since V needs to be flipped to cater for the coordinate system
    differences mentioned before, using a logic that is different for textures
    based on images and textures used as render targets. Fortunately this is all
    taken care of by the engine so the shader need no further logic for this.

    Once simpleEffect is available, it can be associated with the effects list
    of a the View3D's SceneEnvironment:

    \qml
    environment: SceneEnvironment {
        effects: [ simpleEffect ]
    }
    \endqml

    The results would look something like the following, with the original scene
    on the left and with the effect applied on the right:

    \table 70%
    \row
    \li \image effect_intro_1.png
               {Three different 3D objects}
    \li \image effect_intro_2.png
               {Three different 3D objects with transparent Qt logo overlaid as
               fullscreen effect}
    \endtable

    \note The \c shader property value in Shader is a URL, as is the custom in
    QML and Qt Quick, referencing the file containing the shader snippet, and
    works very similarly to ShaderEffect or
    \l{Image::source}{Image.source}. Only the \c file and \c qrc schemes are
    supported.. It is also possible to omit the \c file scheme, allowing to
    specify a relative path in a convenient way. Such a path is resolved
    relative to the component's (the \c{.qml} file's) location.

    \note Shader code is always provided using Vulkan-style GLSL, regardless of
    the graphics API used by Qt at run time.

    \note The vertex and fragment shader code provided by the effect are not
    full, complete GLSL shaders on their own. Rather, they provide a \c MAIN
    function, and optionally a set of \c VARYING declarations, which are then
    amended with further shader code by the engine.

    \note The above example is not compatible with the optional multiview rendering mode that is used in some VR/AR applications.
    To make it function both with and without multiview mode, change MAIN() like this:
    \badcode
    void MAIN()
    {
        vec4 c = texture(tex, TEXTURE_UV);
        c.r *= redLevel;
    #if QSHADER_VIEW_COUNT >= 2
        FRAGCOLOR = c * texture(INPUT, vec3(INPUT_UV, VIEW_INDEX));
    #else
        FRAGCOLOR = c * texture(INPUT, INPUT_UV);
    #endif
    }
    \endcode

    \section1 Effects with vertex shaders

    A vertex shader, when present, must provide a function called \c MAIN. In
    the vast majority of cases the custom vertex shader will not want to provide
    its own calculation of the homogenous vertex position, but it is possible
    using \c POSITION, \c VERTEX, and \c MODELVIEWPROJECTION_MATRIX. When
    \c POSITION is not present in the custom shader code, a statement equivalent to
    \c{POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);} will be
    injected automatically by Qt Quick 3D.

    To pass data between the vertex and fragment shaders, use the VARYING
    keyword. Internally this will then be transformed into the appropriate
    vertex output or fragment input declaration. The fragment shader can use the
    same declaration, which then allows to read the interpolated value for the
    current fragment.

    Let's look at example, that is in effect very similar to the built-in
    DistortionSpiral effect:

    \table 70%
    \row
    \li \badcode
    VARYING vec2 center_vec;
    void MAIN()
    {
        center_vec = INPUT_UV - vec2(0.5, 0.5);
        center_vec.y *= INPUT_SIZE.y / INPUT_SIZE.x;
    }
    \endcode
    \li \badcode
    VARYING vec2 center_vec;
    void MAIN()
    {
        float radius = 0.25;
        float dist_to_center = length(center_vec) / radius;
        vec2 texcoord = INPUT_UV;
        if (dist_to_center <= 1.0) {
            float rotation_amount = (1.0 - dist_to_center) * (1.0 - dist_to_center);
            float r = radians(360.0) * rotation_amount / 4.0;
            mat2 rotation = mat2(cos(r), sin(r), -sin(r), cos(r));
            texcoord = vec2(0.5, 0.5) + rotation * (INPUT_UV - vec2(0.5, 0.5));
        }
        FRAGCOLOR = texture(INPUT, texcoord);
    }
    \endcode
    \endtable

    The Effect object's \c passes list should now specify both the vertex and
    fragment snippets:

    \qml
        passes: Pass {
           shaders: [
               Shader {
                   stage: Shader.Vertex
                   shader: "effect.vert"
               },
               Shader {
                   stage: Shader.Fragment
                   shader: "effect.frag"
               }
            ]
        }
    \endqml

    The end result looks like the following:

    \table 70%
    \row
    \li \image effect_intro_1.png
               {Three different 3D objects}
    \li \image effect_intro_3.png
               {Warped view of three different 3D objects showing vertex shader
               effect}
    \endtable

    \section1 Special keywords in effect shaders

    \list

    \li \c VARYING - Declares a vertex output or fragment input, depending on the type of the current shader.
    \li \c MAIN - This function must always be present in an effect shader.
    \li \c FRAGCOLOR - \c vec4 - The final fragment color; the output of the fragment shader. (fragment shader only)
    \li \c POSITION - \c vec4 - The homogenous position calculated in the vertex shader. (vertex shader only)
    \li \c MODELVIEWPROJECTION_MATRIX - \c mat4 - The transformation matrix for the screen quad.
    \li \c VERTEX - \c vec3 - The vertices of the quad; the input to the vertex shader. (vertex shader only)

    \li \c INPUT - \c sampler2D or \c sampler2DArray - The sampler for the input
    texture with the scene rendered into it, unless a pass redirects its input
    via a BufferInput object, in which case \c INPUT refers to the additional
    color buffer's texture referenced by the BufferInput. With \l{Multiview
    Rendering}{multiview rendering} enabled, which can be relevant for VR/AR
    applications, this is a sampler2DArray, while the input texture becomes a 2D
    texture array.

    \li \c INPUT_UV - \c vec2 - UV coordinates for sampling \c INPUT.

    \li \c TEXTURE_UV - \c vec2 - UV coordinates suitable for sampling a Texture
    with contents loaded from an image file.

    \li \c INPUT_SIZE - \c vec2 - The size of the \c INPUT texture, in pixels.

    \li \c OUTPUT_SIZE - \c vec2 - The size of the output buffer, in
    pixels. Often the same as \c INPUT_SIZE, unless the pass outputs to an extra
    Buffer with a size multiplier on it.

    \li \c FRAME - \c float - A frame counter, incremented after each frame in the View3D.

    \li \c DEPTH_TEXTURE - \c sampler2D or \c sampler2DArray - A depth texture
    with the depth buffer contents with the opaque objects in the scene. Like
    with CustomMaterial, the presence of this keyword in the shader triggers
    generating the depth texture automatically.

    \li \c NORMAL_ROUGHNESS_TEXTURE - \c sampler2D - A texture with the
    world-space normals and material roughness of the opaque objects in the
    currently visible portion of the scene. Like with CustomMaterial, the
    presence of this keyword in the shader implies an additional render pass to
    generate the normal texture.

    \li \c VIEW_INDEX - \c uint - With \l{Multiview Rendering}{multiview
    rendering} enabled, this is the current view index, available in both vertex
    and fragment shaders. Always 0 when multiview rendering is not used.

    \li \c PROJECTION_MATRIX - \c mat4, the projection matrix. Note that with
    \l{Multiview Rendering}{multiview rendering}, this is an array of matrices.

    \li \c INVERSE_PROJECTION_MATRIX - \c mat4, the inverse projection matrix.
    Note that with \l{Multiview Rendering}{multiview rendering}, this is an array
    of matrices.

    \li \c VIEW_MATRIX -> \c mat4, the view (camera) matrix.
    Note that with \l{Multiview Rendering}{multiview rendering}, this is an array
    of matrices.

    \li float \c NDC_Y_UP - The value is \c 1 when the Y axis points up in
    normalized device coordinate space, and \c{-1} when the Y axis points down.
    Y pointing down is the case when rendering happens with Vulkan.

    \li float \c FRAMEBUFFER_Y_UP - The value is \c 1 when the Y axis points up
    in the coordinate system for framebuffers (textures), meaning \c{(0, 0)} is
    the bottom-left corner. The value is \c{-1} when the Y axis points down,
    \c{(0, 0)} being the top-left corner.

    \li float \c NEAR_CLIP_VALUE - The value is \c -1 for when the clipping plane
    range's starts at \c -1 and goes to \c 1.  This is true when using OpenGL for
    rendering. For other rendering backends the value of this property will be
    \c 0 meaning the clipping plane range is \c 0 to \c 1.

    \endlist

    \section1 Building multi-pass effects

    A multi-pass effect often uses more than one set of shaders, and takes the
    \l{Pass::output}{output} and \l{Pass::commands}{commands} properties into
    use. Each entry in the passes list translates to a render pass drawing a
    quad into the pass's output texture, while sampling the effect's input texture
    and optionally other textures as well.

    The typical outline of a multi-pass Effect can look like the following:

    \qml
        passes: [
            Pass {
                shaders: [
                    Shader {
                        stage: Shader.Vertex
                        shader: "pass1.vert"
                    },
                    Shader {
                        stage: Shader.Fragment
                        shader: "pass1.frag"
                    }
                    // This pass outputs to the intermediate texture described
                    // by the Buffer object.
                    output: intermediateColorBuffer
                ],
            },
            Pass {
                shaders: [
                    Shader {
                        stage: Shader.Vertex
                        shader: "pass2.vert"
                    },
                    Shader {
                        stage: Shader.Fragment
                        shader: "pass2.frag"
                    }
                    // The output of the last pass needs no redirection, it is
                    // the final result of the effect.
                ],
                commands: [
                    // This pass reads from the intermediate texture, meaning
                    // INPUT in the shader will refer to the texture associated
                    // with the Buffer.
                    BufferInput {
                        buffer: intermediateColorBuffer
                    }
                ]
            }
        ]
    \endqml

    What is \c intermediateColorBuffer?

    \qml
    Buffer {
        id: intermediateColorBuffer
        name: "tempBuffer"
        // format: Buffer.RGBA8
        // textureFilterOperation: Buffer.Linear
        // textureCoordOperation: Buffer.ClampToEdge
    }
    \endqml

    The commented properties are not necessary if the desired values match the
    defaults.

    Internally the presence of this Buffer object and referencing it from the \c
    output property of a Pass leads to creating a texture with a size matching
    the View3D, and so the size of the implicit input and output textures. When
    this is not desired, the \l{Buffer::sizeMultiplier}{sizeMultiplier} property
    can be used to get an intermediate texture with a different size. This can
    lead to the \c INPUT_SIZE and \c OUTPUT_SIZE uniforms in the shader having
    different values.

    By default the Effect cannot count on textures preserving their contents
    between frames. When a new intermediate texture is created, it is cleared to
    \c{vec4(0.0)}. Afterwards, the same texture can be reused for another
    purpose. Therefore, effect passes should always write to the entire texture,
    without making assumptions about their content at the start of the pass.
    There is an exception to this: Buffer objects with
    \l{Buffer::bufferFlags}{bufferFlags} set to Buffer.SceneLifetime. This
    indicates that the texture is permanently associated with a pass of the
    effect and it will not be reused for other purposes. The contents of such
    color buffers is preserved between frames. This is typically used in a
    ping-pong fashion in effects like motion blur: the first pass takes the
    persistent buffer as its input, in addition to the effects main input
    texture, outputting to another intermediate buffer, while the second pass
    outputs to the persistent buffer. This way in the first frame the first pass
    samples an empty (transparent) texture, whereas in subsequent frames it
    samples the output of the second pass from the previous frame. A third pass
    can then blend the effect's input and the second pass' output together.

    The BufferInput command type is used to expose custom texture buffers to the
    render pass.

    For instance, to access \c someBuffer in the render pass shaders under
    the name, \c mySampler, the following can be added to its command list:
    \qml
    BufferInput { buffer: someBuffer; sampler: "mySampler" }
    \endqml

    If the \c sampler name is not specified, \c INPUT will be used as default.

    Buffers can be useful to share intermediate results between render passes.

    To expose preloaded textures to the effect, TextureInput should be used instead.
    These can be defined as properties of the Effect itself, and will automatically
    be accessible to the shaders by their property names.
    \qml
    property TextureInput tex: TextureInput {
        texture: Texture { source: "image.png" }
    }
    \endqml

    Here \c tex is a valid sampler in all shaders of all the passes of the
    effect.

    When it comes to uniform values from properties, all passes in the Effect
    read the same values in their shaders. If necessary it is possible to
    override the value of a uniform just for a given pass. This is achieved by
    adding the \l SetUniformValue command to the list of commands for the pass.

    \note The \l{SetUniformValue::target}{target} of the pass-specific uniform
    value setter can only refer to a name that is the name of a property of the
    effect. It can override the value for a property's corresponding uniform,
    but it cannot introduce new uniforms.

    \section1 Performance considerations

    Be aware of the increased resource usage and potentially reduced performance
    when using post-processing effects. Just like with Qt Quick layers and
    ShaderEffect, rendering the scene into a texture and then using that to
    texture a quad is not a cheap operation, especially on low-end hardware with
    limited fragment processing power. The amount of additional graphics memory
    needed, as well as the increase in GPU load both depend on the size of the
    View3D (which, on embedded devices without a windowing system, may often be
    as big as the screen resolution). Multi-pass effects, as well as applying
    multiple effects increase the resource and performance requirements further.

    Therefore, it is highly advisable to ensure early on in the development
    lifecycle that the targeted device and graphics stack is able to cope with
    the effects included in the design of the 3D scene at the final product's
    screen resolution.

    While unavoidable with techniques that need it, \c DEPTH_TEXTURE implies an
    additional rendering pass to generate the contents of that texture, which
    can also present a hit on less capable hardware. Therefore, use \c
    DEPTH_TEXTURE in the effect's shaders only when essential.

    The complexity of the operations in the shaders is also important. Just like
    with CustomMaterial, a sub-optimal fragment shader can easily lead to
    reduced rendering performance.

    Be cautious with \l{Buffer::sizeMultiplier}{sizeMultiplier in Buffer} when
    values larger than 1 are involved. For example, a multiplier of 4 means
    creating and then rendering to a texture that is 4 times the size of the
    View3D. Just like with shadow maps and multi- or supersampling, the
    increased resource and performance costs can quickly outweigh the benefits
    from better quality on systems with limited GPU power.

    \section1 VR/AR considerations

    When developing applications for virtual or augmented reality by using Qt
    Quick 3D XR, postprocessing effects are functional and available to use.
    However, designers are developers should take special care to understand
    which and what kind of effects make sense in a virtual reality environment.
    Some effects, including some of the built-in ones in
    ExtendedSceneEnvironment or the deprecated Effects module, do not lead to a
    good visual experience in a VR environment, possibly affecting the user
    physically even (causing, for example, motion sickness or dizziness).

    When the more efficient \l{Multiview Rendering}{multiview rendering mode} is
    enabled in a VR/AR application, there is no separate render pass for the
    left and right eye contents. Instead, it all happens in one pass, using a 2D
    texture array with two layers instead of two independent 2D textures. This
    also means that many intermediate buffers, meaning color or depth textures,
    will need to become texture arrays in this mode. This then has implications
    for custom materials and postprocessing effects. Textures such as the input
    texture (\c INPUT) and the depth texture (\c DEPTH_TEXTURE) become 2D texture
    arrays, exposed in the shader as a \c sampler2DArray instead of \c
    sampler2D. This has implications for GLSL functions such as texture(),
    textureLod(), or textureSize(). The UV coordinate is then a vec3, not a
    vec2. Whereas textureSize() returns a vec3, not a vec2. Effects intended to
    function regardless of the rendering mode, can be written with an
    appropriate ifdef:
    \badcode
    #if QSHADER_VIEW_COUNT >= 2
        vec4 c = texture(INPUT, vec3(INPUT_UV, VIEW_INDEX));
    #else
        vec4 c = texture(INPUT, INPUT_UV);
    #endif
    \endcode

    It can also be useful to define macros that handle both cases. For example:
    \badcode
    #if QSHADER_VIEW_COUNT >= 2
    #define SAMPLE_INPUT(uv) texture(INPUT, vec3(uv, VIEW_INDEX))
    #define SAMPLE_DEPTH(uv) texture(DEPTH_TEXTURE, vec3(uv, VIEW_INDEX)).r
    #define PROJECTION PROJECTION_MATRIX[VIEW_INDEX]
    #define INVERSE_PROJECTION INVERSE_PROJECTION_MATRIX[VIEW_INDEX]
    #else
    #define SAMPLE_INPUT(uv) texture(INPUT, uv)
    #define SAMPLE_DEPTH(uv) texture(DEPTH_TEXTURE, uv).r
    #define PROJECTION PROJECTION_MATRIX
    #define INVERSE_PROJECTION INVERSE_PROJECTION_MATRIX
    #endif
    \endcode

    This does not apply to \c NORMAL_ROUGHNESS_TEXTURE that is always a 2D texture,
    even when multiview rendering is active:
    \badcode
    #define SAMPLE_NORMAL(uv) normalize(texture(NORMAL_ROUGHNESS_TEXTURE, uv).rgb)
    \endcode

    \note The presence of keywords such as \c DEPTH_TEXTURE trigger additional
    render passes, and uniforms such as \c INVERSE_PROJECTION_MATRIX are
    calculated and set upon the presence of the keyword in the shader snippet
    anywhere. This is more expensive, both when it comes to performance and
    resource usage. Hence it is recommended to only add such #defines when the
    textures and matrices will really be used in the effect.

    \sa Shader, Pass, Buffer, BufferInput, {Qt Quick 3D - Custom Effect Example}
*/

/*!
    \qmlproperty list Effect::passes
    Contains a list of render \l {Pass}{passes} implemented by the effect.
*/

QQuick3DEffect::QQuick3DEffect(QQuick3DObject *parent)
    : QQuick3DObject(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::Effect)), parent)
{
}

QQmlListProperty<QQuick3DShaderUtilsRenderPass> QQuick3DEffect::passes()
{
    return QQmlListProperty<QQuick3DShaderUtilsRenderPass>(this,
                                                      nullptr,
                                                      QQuick3DEffect::qmlAppendPass,
                                                      QQuick3DEffect::qmlPassCount,
                                                      QQuick3DEffect::qmlPassAt,
                                                      QQuick3DEffect::qmlPassClear);
}

// Default vertex and fragment shader code that is used when no corresponding
// Shader is present in the Effect. These go through the usual processing so
// should use the user-facing builtins.

static const char *default_effect_vertex_shader =
        "void MAIN()\n"
        "{\n"
        "}\n";

static const char *default_effect_fragment_shader =
        "void MAIN()\n"
        "{\n"
        "#if QSHADER_VIEW_COUNT >= 2\n"
        "    FRAGCOLOR = texture(INPUT, vec3(INPUT_UV, VIEW_INDEX));\n"
        "#else\n"
        "    FRAGCOLOR = texture(INPUT, INPUT_UV);\n"
        "#endif\n"
        "}\n";

static inline void insertVertexMainArgs(QByteArray &snippet)
{
    static const char *argKey =  "/*%QT_ARGS_MAIN%*/";
    const int argKeyLen = int(strlen(argKey));
    const int argKeyPos = snippet.indexOf(argKey);
    if (argKeyPos >= 0)
        snippet = snippet.left(argKeyPos) + QByteArrayLiteral("inout vec3 VERTEX") + snippet.mid(argKeyPos + argKeyLen);
}

static inline void resetShaderDependentEffectFlags(QSSGRenderEffect *effectNode)
{
    effectNode->setFlag(QSSGRenderEffect::Flags::UsesDepthTexture, false);
    effectNode->setFlag(QSSGRenderEffect::Flags::UsesProjectionMatrix, false);
    effectNode->setFlag(QSSGRenderEffect::Flags::UsesInverseProjectionMatrix, false);
    effectNode->setFlag(QSSGRenderEffect::Flags::UsesViewMatrix, false);
    effectNode->setFlag(QSSGRenderEffect::Flags::UsesNormalTexture, false);
    effectNode->setFlag(QSSGRenderEffect::Flags::UsesMotionVectorTexture, false);
}

static inline void accumulateEffectFlagsFromShader(QSSGRenderEffect *effectNode, const QSSGCustomShaderMetaData &meta)
{
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesDepthTexture))
        effectNode->setFlag(QSSGRenderEffect::Flags::UsesDepthTexture);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesProjectionMatrix))
        effectNode->setFlag(QSSGRenderEffect::Flags::UsesProjectionMatrix);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesInverseProjectionMatrix))
        effectNode->setFlag(QSSGRenderEffect::Flags::UsesInverseProjectionMatrix);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesViewMatrix))
        effectNode->setFlag(QSSGRenderEffect::Flags::UsesViewMatrix);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesNormalTexture))
        effectNode->setFlag(QSSGRenderEffect::Flags::UsesNormalTexture);
    if (meta.flags.testFlag(QSSGCustomShaderMetaData::UsesMotionVectorTexture))
        effectNode->setFlag(QSSGRenderEffect::Flags::UsesMotionVectorTexture);
}

QSSGRenderGraphObject *QQuick3DEffect::updateSpatialNode(QSSGRenderGraphObject *node)
{
    using namespace QSSGShaderUtils;

    const auto &renderContext = QQuick3DObjectPrivate::get(this)->sceneManager->wattached->rci();
    if (!renderContext) {
        qWarning("QQuick3DEffect: No render context interface?");
        return nullptr;
    }

    QSSGRenderEffect *effectNode = static_cast<QSSGRenderEffect *>(node);
    bool newBackendNode = false;
    if (!effectNode) {
        effectNode = new QSSGRenderEffect;
        newBackendNode = true;
    }

    bool shadersOrBuffersMayChange = false;
    if (m_dirtyAttributes & Dirty::EffectChainDirty)
        shadersOrBuffersMayChange = true;

    const bool fullUpdate = newBackendNode || effectNode->incompleteBuildTimeObject || (m_dirtyAttributes & Dirty::TextureDirty);

    if (fullUpdate || shadersOrBuffersMayChange) {
        markAllDirty();

        // Need to clear the old list with properties and textures first.
        effectNode->properties.clear();
        effectNode->textureProperties.clear();

        QMetaMethod propertyDirtyMethod;
        const int idx = metaObject()->indexOfSlot("onPropertyDirty()");
        if (idx != -1)
            propertyDirtyMethod = metaObject()->method(idx);

        // Properties -> uniforms
        QSSGShaderCustomMaterialAdapter::StringPairList uniforms;
        QSSGShaderCustomMaterialAdapter::StringPairList multiViewDependentSamplers;
        const int propCount = metaObject()->propertyCount();
        int propOffset = metaObject()->propertyOffset();

        // Effect can have multilayered inheritance structure, so find the actual propOffset
        const QMetaObject *superClass = metaObject()->superClass();
        while (superClass && qstrcmp(superClass->className(), "QQuick3DEffect") != 0)  {
            propOffset = superClass->propertyOffset();
            superClass = superClass->superClass();
        }

        using TextureInputProperty = QPair<QQuick3DShaderUtilsTextureInput *, const char *>;

        QVector<TextureInputProperty> textureProperties; // We'll deal with these later
        for (int i = propOffset; i != propCount; ++i) {
            const QMetaProperty property = metaObject()->property(i);
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
                    effectNode->properties.push_back({ name, uniformTypeName(propType),
                                                       propValue, uniformType(propType), i});
                    // Track the property changes
                    if (fullUpdate) {
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
            QSSGRenderEffect::TextureProperty texProp;
            QQuick3DTexture *tex = texture.texture(); // may be null if the TextureInput has no 'texture' set
            if (fullUpdate) {
                connect(&texture, &QQuick3DShaderUtilsTextureInput::enabledChanged, this, &QQuick3DEffect::onTextureDirty);
                connect(&texture, &QQuick3DShaderUtilsTextureInput::textureChanged, this, &QQuick3DEffect::onTextureDirty);
            } // else already connected
            texProp.name = name;
            if (texture.enabled && tex)
                texProp.texImage = tex->getRenderImage();

            texProp.shaderDataType = QSSGRenderShaderValue::Texture;

            if (tex) {
                texProp.minFilterType = tex->minFilter() == QQuick3DTexture::Nearest ? QSSGRenderTextureFilterOp::Nearest
                                                                                     : QSSGRenderTextureFilterOp::Linear;
                texProp.magFilterType = tex->magFilter() == QQuick3DTexture::Nearest ? QSSGRenderTextureFilterOp::Nearest
                                                                                     : QSSGRenderTextureFilterOp::Linear;
                texProp.mipFilterType = tex->generateMipmaps() ? (tex->mipFilter() == QQuick3DTexture::Nearest ? QSSGRenderTextureFilterOp::Nearest
                                                                                                               : QSSGRenderTextureFilterOp::Linear)
                                                               : QSSGRenderTextureFilterOp::None;
                texProp.horizontalClampType = tex->horizontalTiling() == QQuick3DTexture::Repeat ? QSSGRenderTextureCoordOp::Repeat
                                                                                                 : (tex->horizontalTiling() == QQuick3DTexture::ClampToEdge ? QSSGRenderTextureCoordOp::ClampToEdge
                                                                                                                                                            : QSSGRenderTextureCoordOp::MirroredRepeat);
                texProp.verticalClampType = tex->verticalTiling() == QQuick3DTexture::Repeat ? QSSGRenderTextureCoordOp::Repeat
                                                                                             : (tex->verticalTiling() == QQuick3DTexture::ClampToEdge ? QSSGRenderTextureCoordOp::ClampToEdge
                                                                                                                                                      : QSSGRenderTextureCoordOp::MirroredRepeat);
                texProp.zClampType = tex->depthTiling() == QQuick3DTexture::Repeat ? QSSGRenderTextureCoordOp::Repeat
                        : (tex->depthTiling() == QQuick3DTexture::ClampToEdge)     ? QSSGRenderTextureCoordOp::ClampToEdge
                                                                           : QSSGRenderTextureCoordOp::MirroredRepeat;
            }

            // Knowing upfront that a sampler2D needs to be a sampler2DArray in
            // the multiview-compatible version of the shader is not trivial.
            // Consider: we know the list of TextureInputs, without any
            // knowledge about the usage of those textures. Intermediate buffers
            // (textures) also have a default constructed (no source, no source
            // item, no texture data) Texture set. What indicates that these are
            // used as intermediate buffers, is the 'output' property of a Pass,
            // referencing a Buffer object (which objects we otherwise do not
            // track), the 'name' of which matches TextureInput property name.
            // The list of passes may vary dynamically, and some Passes may not
            // be listed at any point in time if the effect has an
            // ubershader-ish design. Thus one can have TextureInputs that are
            // not associated with a Buffer (when scanning through the Passes),
            // and so we cannot just check the 'output'-referenced Buffers to
            // decide if a TextureInput's Texture needs to be treated specially
            // in the generated shader code. (and the type must be correct even
            // for, from our perspective, "unused" samplers since they are still
            // in the shader code, and will get a dummy texture bound)
            //
            // Therefore, in the absence of more sophisticated options, we just
            // look at the TextureInput's texture, and if it is something along
            // the lines of
            //    property TextureInput intermediateColorBuffer1: TextureInput { texture: Texture { } }
            // then it is added to the special list, indicating the the type is
            // sampler2D or sampler2DArray, depending on the rendering mode the
            // shader is targeting.

            if (tex && !tex->hasSourceData()) {
                multiViewDependentSamplers.append({ QByteArrayLiteral("sampler2D"), name }); // the type may get adjusted later
            } else {
                if (tex && QQuick3DObjectPrivate::get(tex)->type == QQuick3DObjectPrivate::Type::ImageCube)
                    uniforms.append({ QByteArrayLiteral("samplerCube"), name });
                else if (tex && tex->textureData() && tex->textureData()->depth() > 0)
                    uniforms.append({ QByteArrayLiteral("sampler3D"), name });
                else
                    uniforms.append({ QByteArrayLiteral("sampler2D"), name });
            }

            effectNode->textureProperties.push_back(texProp);
        };

        // Textures
        for (const auto &property : std::as_const(textureProperties))
            processTextureProperty(*property.first, property.second);

        if (effectNode->incompleteBuildTimeObject) { // This object came from the shadergen tool
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
                        effectNode->properties.push_back({ name, uniformTypeName(propType),
                                                           propValue, uniformType(propType), -1 /* aka. dynamic property */});
                        // We don't need to track property changes
                    } else {
                        // ### figure out how _not_ to warn when there are no dynamic
                        // properties defined (because warnings like Blah blah objectName etc. are not helpful)
                        qWarning("No known uniform conversion found for effect property %s. Skipping", name.constData());
                    }
                }
            }

            for (const auto &property : std::as_const(textureProperties))
                processTextureProperty(*property.first, property.second);
        }

        // built-ins
        uniforms.append({ "mat4", "qt_modelViewProjection" });
        uniforms.append({ "vec2", "qt_inputSize" });
        uniforms.append({ "vec2", "qt_outputSize" });
        uniforms.append({ "float", "qt_frame_num" });
        uniforms.append({ "float", "qt_fps" });
        uniforms.append({ "vec2", "qt_cameraProperties" });
        uniforms.append({ "float", "qt_normalAdjustViewportFactor" });
        uniforms.append({ "float", "qt_nearClipValue" });
        uniforms.append({ "vec4", "qt_rhi_properties" });

        // qt_inputTexture is not listed in uniforms, will be added by prepareCustomShader()
        // since the name and type varies between non-multiview and multiview mode

        QSSGShaderCustomMaterialAdapter::StringPairList builtinVertexInputs;
        builtinVertexInputs.append({ "vec3", "attr_pos" });
        builtinVertexInputs.append({ "vec2", "attr_uv" });

        QSSGShaderCustomMaterialAdapter::StringPairList builtinVertexOutputs;
        builtinVertexOutputs.append({ "vec2", "qt_inputUV" });
        builtinVertexOutputs.append({ "vec2", "qt_textureUV" });
        builtinVertexOutputs.append({ "flat uint", "qt_viewIndex" });

        // fragOutput is added automatically by the program generator

        resetShaderDependentEffectFlags(effectNode);

        if (!m_passes.isEmpty()) {
            const QQmlContext *context = qmlContext(this);
            effectNode->resetCommands();
            for (QQuick3DShaderUtilsRenderPass *pass : std::as_const(m_passes)) {
                // Have a key composed more or less of the vertex and fragment filenames.
                // The shaderLibraryManager uses stage+shaderPathKey as the key.
                // Thus shaderPathKey is then sufficient to look up both the vertex and fragment shaders later on.
                // Note that this key is not suitable as a unique key for the graphics resources because the same
                // set of shader files can be used in multiple different passes, or in multiple active effects.
                // But that's the effect system's problem.
                QByteArray shaderPathKey("effect pipeline--");
                QSSGRenderEffect::ShaderPrepPassData passData;
                for (QQuick3DShaderUtilsShader::Stage stage : { QQuick3DShaderUtilsShader::Stage::Vertex, QQuick3DShaderUtilsShader::Stage::Fragment }) {
                    QQuick3DShaderUtilsShader *shader = nullptr;
                    for (QQuick3DShaderUtilsShader *s : pass->m_shaders) {
                        if (s->stage == stage) {
                            shader = s;
                            break;
                        }
                    }

                    // just how many enums does one need for the exact same thing...
                    QSSGShaderCache::ShaderType type = QSSGShaderCache::ShaderType::Vertex;
                    if (stage == QQuick3DShaderUtilsShader::Stage::Fragment)
                        type = QSSGShaderCache::ShaderType::Fragment;

                    // Will just use the custom material infrastructure. Some
                    // substitutions are common between custom materials and effects.
                    //
                    // Substitutions relevant to us here:
                    //   MAIN -> qt_customMain
                    //   FRAGCOLOR -> fragOutput
                    //   POSITION -> gl_Position
                    //   MODELVIEWPROJECTION_MATRIX -> qt_modelViewProjection
                    //   DEPTH_TEXTURE -> qt_depthTexture
                    //   ... other things shared with custom material
                    //
                    //   INPUT -> qt_inputTexture
                    //   INPUT_UV -> qt_inputUV
                    //   ... other effect specifics
                    //
                    // Built-in uniforms, inputs and outputs will be baked into
                    // metadata comment blocks in the resulting source code.
                    // Same goes for inputs/outputs declared with VARYING.

                    QByteArray code;
                    if (shader) {
                        code = QSSGShaderUtils::resolveShader(shader->shader, context, shaderPathKey); // appends to shaderPathKey
                    } else {
                        if (!shaderPathKey.isEmpty())
                            shaderPathKey.append('>');
                        shaderPathKey += "DEFAULT";
                        if (type == QSSGShaderCache::ShaderType::Vertex)
                            code = default_effect_vertex_shader;
                        else
                            code = default_effect_fragment_shader;
                    }

                    for (auto pathKeyIndex : { QSSGRenderCustomMaterial::RegularShaderPathKeyIndex, QSSGRenderCustomMaterial::MultiViewShaderPathKeyIndex }) {
                        QSSGShaderCustomMaterialAdapter::ShaderCodeAndMetaData result;
                        QSSGShaderCustomMaterialAdapter::CustomShaderPrepWorkData scratch;

                        QSSGShaderCustomMaterialAdapter::beginPrepareCustomShader(
                            &scratch,
                            &result,
                            code,
                            type,
                            pathKeyIndex == QSSGRenderCustomMaterial::RegularShaderPathKeyIndex ? false : true);

                        QSSGShaderCustomMaterialAdapter::StringPairList multiViewDependentUniforms;
                        if (result.second.flags.testFlag(QSSGCustomShaderMetaData::UsesProjectionMatrix)
                            || result.second.flags.testFlag(QSSGCustomShaderMetaData::UsesInverseProjectionMatrix))
                        {
                            multiViewDependentUniforms.append({ "mat4", "qt_projectionMatrix" });
                            multiViewDependentUniforms.append({ "mat4", "qt_inverseProjectionMatrix" });
                        }

                        multiViewDependentUniforms.append({ "mat4", "qt_viewMatrix" });

                        accumulateEffectFlagsFromShader(effectNode, result.second);

                        QSSGShaderCustomMaterialAdapter::finishPrepareCustomShader(
                            &result.first, // effectively appends the QQ3D_SHADER_META block
                            scratch,
                            result,
                            type,
                            pathKeyIndex == QSSGRenderCustomMaterial::RegularShaderPathKeyIndex ? false : true,
                            uniforms,
                            type == QSSGShaderCache::ShaderType::Vertex ? builtinVertexInputs : builtinVertexOutputs,
                            type == QSSGShaderCache::ShaderType::Vertex ? builtinVertexOutputs : QSSGShaderCustomMaterialAdapter::StringPairList(),
                            multiViewDependentSamplers,
                            multiViewDependentUniforms);

                        if (type == QSSGShaderCache::ShaderType::Vertex) {
                            // qt_customMain() has an argument list which gets injected here
                            insertVertexMainArgs(result.first);
                            passData.vertexShaderCode[pathKeyIndex] = result.first;
                            passData.vertexMetaData[pathKeyIndex] = result.second;
                        } else {
                            passData.fragmentShaderCode[pathKeyIndex] = result.first;
                            passData.fragmentMetaData[pathKeyIndex] = result.second;
                        }
                    }
                }

                effectNode->commands.push_back(nullptr); // will be changed to QSSGBindShader in finalizeShaders
                passData.bindShaderCmdIndex = effectNode->commands.size() - 1;

                // finalizing the shader code happens in a separate step later on by the backend node
                passData.shaderPathKeyPrefix = shaderPathKey;
                effectNode->shaderPrepData.passes.append(passData);
                effectNode->shaderPrepData.valid = true; // trigger reprocessing the shader code later on

                effectNode->commands.push_back(new QSSGApplyInstanceValue);

                // Buffers
                QQuick3DShaderUtilsBuffer *outputBuffer = pass->outputBuffer;
                if (outputBuffer) {
                    const QByteArray &outBufferName = outputBuffer->name;
                    if (outBufferName.isEmpty()) {
                        // default output buffer (with settings)
                        auto outputFormat = QQuick3DShaderUtilsBuffer::mapTextureFormat(outputBuffer->format());
                        effectNode->commands.push_back(new QSSGBindTarget(outputFormat));
                        effectNode->outputFormat = outputFormat;
                    } else {
                        // Allocate buffer command
                        effectNode->commands.push_back(outputBuffer->cloneCommand());
                        connect(outputBuffer, &QQuick3DShaderUtilsBuffer::changed, this, &QQuick3DEffect::onPassDirty, Qt::UniqueConnection);
                        // bind buffer
                        effectNode->commands.push_back(new QSSGBindBuffer(outBufferName));
                    }
                } else {
                    // Use the default output buffer, same format as the source buffer
                    effectNode->commands.push_back(new QSSGBindTarget(QSSGRenderTextureFormat::Unknown));
                    effectNode->outputFormat = QSSGRenderTextureFormat::Unknown;
                }

                // Other commands (BufferInput, Blending ... )
                const auto &extraCommands = pass->m_commands;
                for (const auto &command : extraCommands) {
                    const int bufferCount = command->bufferCount();
                    for (int i = 0; i != bufferCount; ++i) {
                        effectNode->commands.push_back(command->bufferAt(i)->cloneCommand());
                        connect(command->bufferAt(i), &QQuick3DShaderUtilsBuffer::changed, this, &QQuick3DEffect::onPassDirty, Qt::UniqueConnection);
                    }
                    effectNode->commands.push_back(command->cloneCommand());
                }

                effectNode->commands.push_back(new QSSGRender);
            }
        }
    }

    if (m_dirtyAttributes & Dirty::PropertyDirty) {
        for (const auto &prop : std::as_const(effectNode->properties)) {
            auto p = metaObject()->property(prop.pid);
            if (Q_LIKELY(p.isValid()))
                prop.value = p.read(this);
        }
    }

    m_dirtyAttributes = 0;

    DebugViewHelpers::ensureDebugObjectName(effectNode, this);

    return effectNode;
}

void QQuick3DEffect::onPropertyDirty()
{
    markDirty(Dirty::PropertyDirty);
}

void QQuick3DEffect::onTextureDirty()
{
    markDirty(Dirty::TextureDirty);
}

void QQuick3DEffect::onPassDirty()
{
    // changed() signals from not just Passes but Buffers are also hooked up to this.
    // Any property change should lead to re-evaluating the whole effect in sync.
    markDirty(Dirty::EffectChainDirty);
}

void QQuick3DEffect::effectChainDirty()
{
    markDirty(Dirty::EffectChainDirty);
}

void QQuick3DEffect::markDirty(QQuick3DEffect::Dirty type)
{
    if (!(m_dirtyAttributes & quint32(type))) {
        m_dirtyAttributes |= quint32(type);
        update();
    }
}

void QQuick3DEffect::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    if (sceneManager) {
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

void QQuick3DEffect::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

void QQuick3DEffect::qmlAppendPass(QQmlListProperty<QQuick3DShaderUtilsRenderPass> *list, QQuick3DShaderUtilsRenderPass *pass)
{
    if (!pass)
        return;

    QQuick3DEffect *that = qobject_cast<QQuick3DEffect *>(list->object);
    that->m_passes.push_back(pass);

    connect(pass, &QQuick3DShaderUtilsRenderPass::changed, that, &QQuick3DEffect::onPassDirty);
    that->effectChainDirty();
}

QQuick3DShaderUtilsRenderPass *QQuick3DEffect::qmlPassAt(QQmlListProperty<QQuick3DShaderUtilsRenderPass> *list, qsizetype index)
{
    QQuick3DEffect *that = qobject_cast<QQuick3DEffect *>(list->object);
    return that->m_passes.at(index);
}

qsizetype QQuick3DEffect::qmlPassCount(QQmlListProperty<QQuick3DShaderUtilsRenderPass> *list)
{
    QQuick3DEffect *that = qobject_cast<QQuick3DEffect *>(list->object);
    return that->m_passes.size();
}

void QQuick3DEffect::qmlPassClear(QQmlListProperty<QQuick3DShaderUtilsRenderPass> *list)
{
    QQuick3DEffect *that = qobject_cast<QQuick3DEffect *>(list->object);

    for (QQuick3DShaderUtilsRenderPass *pass : that->m_passes)
        pass->disconnect(that);

    that->m_passes.clear();
    that->effectChainDirty();
}

void QQuick3DEffect::setDynamicTextureMap(QQuick3DShaderUtilsTextureInput *textureMap)
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

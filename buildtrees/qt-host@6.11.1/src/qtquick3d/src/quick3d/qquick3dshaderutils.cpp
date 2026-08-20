// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dshaderutils_p.h"

#include <QtCore/qfile.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlfile.h>

#include "qquick3dviewport_p.h"
#include "qquick3dcustommaterial_p.h"
#include "qquick3deffect_p.h"
#include "qquick3dtextureproviderextension.h"
#include <ssg/qssgrenderextensions.h>
#include <ssg/qquick3dextensionhelpers.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderimage_p.h>

#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSubRenderPass, "qt.quick3d.subrenderpass")

/*!
    \qmltype Shader
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Container component for defining shader code used by post-processing effects.

    The Shader type is used for populating the \l{Pass::shaders}{shaders} list in the
    render \l{Pass}{pass} of an \l Effect.

    A shader is code which is executed directly on the graphic hardware at a particular
    \l{Shader::stage}{stage} of the rendering pipeline.

    \sa Effect
*/
/*!
    \qmlproperty url Shader::shader
    Specifies the name of the shader source file. For details on how to write shader code,
    see the \l Effect documentation.

    \warning Shader snippets are assumed to be trusted content. Application
    developers are advised to carefully consider the potential implications
    before allowing the loading of user-provided content that is not part of the
    application.
*/
/*!
    \qmlproperty enumeration Shader::stage
    Specifies the stage of the rendering pipeline when the shader code will be executed.
    The default is \c Shader.Fragment

    \value Shader.Vertex The shader is a vertex shader. This code is run once per vertex
    in the input geometry and can be used to modify it before the geometry is rasterized
    (scan converted). In the case of effects, the input geometry is always a quad (four
    vertexes representing the corners of the render target).
    \value Shader.Fragment The shader is a fragment shader. After vertex processing,
    the modified geometry is turned into fragments (rasterization). Then a fragment shader
    is executed for each fragment, assigning a color to it. Fragments are a related concept
    to pixels, but with additional information attached. Also, as a result of some
    anti-aliasing strategies, there may be more than one fragment for each pixel in the
    output.
*/

/*!
    \qmltype TextureInput
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Specifies a texture exposed to the shaders of a CustomMaterial or Effect.

    This is a type which can be used for exposing a \l Texture to a shader, either
    in the \l{Pass}{render pass} of an \l Effect, or in a \l CustomMaterial. It exists
    primarily to assign a local name to the \l Texture that can be referenced from
    shaders.

    When a TextureInput property is declared in an \l Effect or a \l CustomMaterial,
    it will automatically be available as a sampler in all shaders by its property
    name.
*/
/*!
    \qmlproperty Texture TextureInput::texture
    The texture for which this TextureInput serves as an indirection.
*/
/*!
    \qmlproperty bool TextureInput::enabled
    The property determines if this TextureInput is enabled. The default value
    is true. When disabled, the shaders of the effect sample a dummy, opaque
    black texture instead of the one specified by \l texture.
*/

/*!
    \qmltype Pass
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Defines a render pass in an Effect.

    An \l Effect may consist of multiple render passes. Each render pass has a
    setup phase where the list of \l{Pass::commands}{render commands} are executed,
    a \l{Pass::output}{output buffer} and a list of \l{Pass::shaders}{shaders} to
    use for rendering the effect.

    See the documentation for \l Effect for more details on how to set up multiple
    rendering passes.
*/
/*!
    \qmlproperty Buffer Pass::output
    Specifies the output \l {Buffer}{buffer} of the pass.
*/
/*!
    \qmlproperty list Pass::commands
    Specifies the list of render \l {Command}{commands} of the pass.
*/
/*!
    \qmlproperty list Pass::shaders
    Specifies the list of \l {Shader}{shaders} of the pass.
*/

/*!
    \qmltype Command
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Supertype of commands to be performed as part of a pass in an Effect.

    The Command type should not be instantiated by itself, but only exists as a
    polymorphic supertype for the different actions that can be performed as part
    of a \l{Pass}{render pass}.

    \sa BufferInput, SetUniformValue, Effect
*/

/*!
    \qmltype BufferInput
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Defines an input buffer to be used as input for a pass of an Effect.

    BufferInput is a \l Command which can be added to the list of commands in the \l Pass of
    an \l Effect. When executed, it will expose the buffer as a sample to the shaders
    in the render pass. The shaders must declare a sampler with the name given in the
    BufferInput's \c sampler property.

    This can be used for sharing intermediate results between the different passes of an
    effect.

    \sa TextureInput
*/
/*!
    \qmlproperty Buffer BufferInput::buffer
    Specifies the \l {Buffer}{buffer} which should be exposed to the shader.
*/
/*!
    \qmlproperty string BufferInput::sampler
    Specifies the name under which the buffer is exposed to the shader.
    When this property is not set, the buffer is exposed with the built-in name \c INPUT.
*/

/*!
    \qmltype Buffer
    \inherits QtObject
    \inqmlmodule QtQuick3D
    \brief Creates or references a color buffer to be used for a pass of an Effect.

    A Buffer can be used to create intermediate buffers to share data between
    \l{Pass}{render passes} in an \l Effect.

    \note If the \l name property of the Buffer is empty, it will reference the
    default output texture of the render pass.
*/
/*!
    \qmlproperty enumeration Buffer::format
    Specifies the texture format. The default value is Buffer.RGBA8.

    \value Buffer.RGBA8
    \value Buffer.RGBA16F
    \value Buffer.RGBA32F
    \value Buffer.R8
    \value Buffer.R16
    \value Buffer.R16F
    \value Buffer.R32F
*/
/*!
    \qmlproperty enumeration Buffer::textureFilterOperation
    Specifies the texture filtering mode when sampling the contents of the
    Buffer. The default value is Buffer.Linear.

    \value Buffer.Nearest Use nearest-neighbor filtering.
    \value Buffer.Linear Use linear filtering.
*/
/*!
    \qmlproperty enumeration Buffer::textureCoordOperation
    Specifies the behavior for texture coordinates when sampling outside the [0, 1] range.
    The default is Buffer.ClampToEdge.

    \value Buffer.ClampToEdge Clamp coordinates to the edges.
    \value Buffer.Repeat Wrap the coordinates at the edges to tile the texture.
    \value Buffer.MirroredRepeat Wrap the coordinate at the edges, but mirror the texture
    when tiling it.
*/
/*!
    \qmlproperty real Buffer::sizeMultiplier
    Specifies the size multiplier of the buffer. For instance, a value of \c 1.0 creates
    a buffer with the same size as the effect's input texture while \c 0.5 creates buffer
    where both width and height is half as big. The default value is 1.0.
*/
/*!
    \qmlproperty enumeration Buffer::bufferFlags
    Specifies the buffer allocation flags. The default is Buffer.None.

    \value Buffer.None No special behavior.
    \value Buffer.SceneLifetime The buffer is allocated for the whole lifetime of the scene.
*/
/*!
    \qmlproperty string Buffer::name
    Specifies the name of the buffer.

    \note When this property is empty, the Buffer will refer to the default output texture
    of the \l{Pass}{render pass} instead of allocating a buffer. This can be useful to
    override certain settings of the output, such as the texture format, without introducing
    a new, separate intermediate texture.
*/

/*!
    \qmltype SetUniformValue
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Defines a value to be set during a single \l {Pass}{pass}.
    \since 5.15

    SetUniformValue is a \l Command which can be added to the list of commands in a \l Pass. When
    executed, it will set the uniform given by the \l{SetUniformValue::target}{target} property
    to \l{SetUniformValue::value}{value}.

    \note The value set by this command is will only be set during the \l {Pass}{pass} it occurs in.
    For consecutive passes the value will be revert to the initial value of the uniform as it
    was defined in the \l Effect item.

    \sa BufferInput
*/
/*!
    \qmlproperty string SetUniformValue::target
    Specifies the name of the uniform that will have its value changed during the \l {Pass}{pass}.
    This must match the name of an existing property in the \l Effect.
*/
/*!
    \qmlproperty Variant SetUniformValue::value
    Specifies the value that will be set on the \c target uniform.
*/

/*!
    \qmlvaluetype renderTargetBlend
    \inqmlmodule QtQuick3D
    \brief Defines blending parameters for a single color attachment of a render pass.
    \since 6.11

    The renderTargetBlend type is used to specify blending parameters for a single
    color attachment of a \l RenderPass. An instance of renderTargetBlend can be
    assigned to one of the \l PipelineStateOverride \c targetBlend0 through \c targetBlend7
    properties, where the index corresponds to the color attachment slot.

    The color blend equation is:
    \badcode
    result.rgb = srcColor * src.rgb  opColor  dstColor * dst.rgb
    result.a   = srcAlpha * src.a   opAlpha  dstAlpha * dst.a
    \endcode
*/

/*!
    \qmlproperty bool renderTargetBlend::blendEnabled
    If set to \c true, enables blending for the color attachment. If set to \c false,
    disables blending and the output color is written directly to the attachment.
    The default value is \c false.
*/

/*!
    \qmlproperty enumeration renderTargetBlend::colorWrite
    A bitmask that controls which color channels are written to the color attachment.
    Multiple values can be combined with the \c | operator.
    The default value is \c {R | G | B | A}.

    \value RenderTargetBlend.R Write to the red channel.
    \value RenderTargetBlend.G Write to the green channel.
    \value RenderTargetBlend.B Write to the blue channel.
    \value RenderTargetBlend.A Write to the alpha channel.
*/

/*!
    \qmlproperty enumeration renderTargetBlend::srcColor
    Sets the source color blend factor. This value scales the RGB components of the
    fragment shader output before the blend operation. The default value is \c One.

    \value RenderTargetBlend.Zero Factor is (0, 0, 0).
    \value RenderTargetBlend.One Factor is (1, 1, 1).
    \value RenderTargetBlend.SrcColor Factor is the source color (r, g, b).
    \value RenderTargetBlend.OneMinusSrcColor Factor is (1-r, 1-g, 1-b) of the source.
    \value RenderTargetBlend.DstColor Factor is the destination color (r, g, b).
    \value RenderTargetBlend.OneMinusDstColor Factor is (1-r, 1-g, 1-b) of the destination.
    \value RenderTargetBlend.SrcAlpha Factor is the source alpha (a, a, a).
    \value RenderTargetBlend.OneMinusSrcAlpha Factor is (1-a, 1-a, 1-a) of the source.
    \value RenderTargetBlend.DstAlpha Factor is the destination alpha (a, a, a).
    \value RenderTargetBlend.OneMinusDstAlpha Factor is (1-a, 1-a, 1-a) of the destination.
    \value RenderTargetBlend.ConstantColor Factor is the constant color set on the pipeline.
    \value RenderTargetBlend.OneMinusConstantColor Factor is one minus the constant color.
    \value RenderTargetBlend.ConstantAlpha Factor is the constant alpha.
    \value RenderTargetBlend.OneMinusConstantAlpha Factor is one minus the constant alpha.
    \value RenderTargetBlend.SrcAlphaSaturate Factor is min(src.a, 1-dst.a) for RGB, 1 for A.
    \value RenderTargetBlend.Src1Color Factor is from the second color output of the fragment shader.
    \value RenderTargetBlend.OneMinusSrc1Color Factor is one minus the second color output.
    \value RenderTargetBlend.Src1Alpha Factor is the alpha of the second color output.
    \value RenderTargetBlend.OneMinusSrc1Alpha Factor is one minus the alpha of the second output.
*/

/*!
    \qmlproperty enumeration renderTargetBlend::dstColor
    Sets the destination color blend factor. This value scales the RGB components of the
    current value in the color attachment before the blend operation.
    The default value is \c OneMinusSrcAlpha.
    See \c srcColor for the list of available values.
*/

/*!
    \qmlproperty enumeration renderTargetBlend::opColor
    Sets the arithmetic operation used to combine the scaled source and destination
    color components. The default value is \c Add.

    \value RenderTargetBlend.Add Result = src + dst.
    \value RenderTargetBlend.Subtract Result = src - dst.
    \value RenderTargetBlend.ReverseSubtract Result = dst - src.
    \value RenderTargetBlend.Min Result = min(src, dst).
    \value RenderTargetBlend.Max Result = max(src, dst).
*/

/*!
    \qmlproperty enumeration renderTargetBlend::srcAlpha
    Sets the source alpha blend factor. This value scales the alpha component of the
    fragment shader output before the blend operation. The default value is \c One.
    See \c srcColor for the list of available values.
*/

/*!
    \qmlproperty enumeration renderTargetBlend::dstAlpha
    Sets the destination alpha blend factor. This value scales the alpha component of the
    current value in the color attachment before the blend operation.
    The default value is \c OneMinusSrcAlpha.
    See \c srcColor for the list of available values.
*/

/*!
    \qmlproperty enumeration renderTargetBlend::opAlpha
    Sets the arithmetic operation used to combine the scaled source and destination
    alpha components. The default value is \c Add.
    See \c opColor for the list of available values.
*/

/*!
    \qmltype PipelineStateOverride
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Defines pipeline state overrides for a single \l {RenderPass}{pass}.
    \since 6.11

    PipelineStateOverride is a \l Command which can be added to the list of commands in a \l RenderPass.
    When executed, it will override the pipeline state in the render pass according to the properties set on
    the PipelineStateOverride. Only values that are set will override the existing pipeline state's values.
    If you want to reset a value that has been overridden to the default, then set the property
    to \c undefined.

    \sa renderTargetBlend
*/

/*!
    \qmltype DepthStencilAttachment
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Defines a depth-stencil attachment for a RenderPass.
    \since 6.11

    DepthStencilAttachment adds an implicit depth/stencil render buffer to a
    \l RenderPass. The render buffer is automatically created and managed by the
    rendering system. Because it is backed by an opaque render buffer rather
    than a texture, its contents cannot be sampled in shaders. If you need to
    read back depth values in a later pass, use \l DepthTextureAttachment instead.

    Use DepthStencilAttachment when you need depth testing and/or stencil
    operations in a pass but do not need to access the depth data afterwards.

    \qml
    RenderPass {
        commands: [
            ColorAttachment { name: "color0"; target: colorTexture },
            // Add a depth buffer so depth testing works in this pass
            DepthStencilAttachment {}
        ]
    }
    \endqml

    \sa DepthTextureAttachment, RenderPassTexture
*/

namespace QSSGShaderUtils {

ResolveFunction resolveShaderOverride = nullptr;

void setResolveFunction(ResolveFunction fn)
{
    resolveShaderOverride = fn;
}

QByteArray resolveShader(const QUrl &fileUrl, const QQmlContext *context, QByteArray &shaderPathKey)
{
    if (resolveShaderOverride) {
        QByteArray shaderData;
        if (resolveShaderOverride(fileUrl, context, shaderData, shaderPathKey))
            return shaderData;
    }

    if (!shaderPathKey.isEmpty())
        shaderPathKey.append('>');

    const QUrl loadUrl = context ? context->resolvedUrl(fileUrl) : fileUrl;
    const QString filePath = QQmlFile::urlToLocalFileOrQrc(loadUrl);

    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        shaderPathKey += loadUrl.fileName().toUtf8();
        return f.readAll();
    } else {
        qWarning("Failed to read shader code from %s", qPrintable(filePath));
    }

    return QByteArray();
}

// These are the QMetaTypes that we convert into uniforms.
static constexpr QMetaType::Type qssg_metatype_list[] {
        QMetaType::Double,
        QMetaType::Bool,
        QMetaType::QVector2D,
        QMetaType::QVector3D,
        QMetaType::QVector4D,
        QMetaType::Int,
        QMetaType::QColor,
        QMetaType::QSize,
        QMetaType::QSizeF,
        QMetaType::QPoint,
        QMetaType::QPointF,
        QMetaType::QRect,
        QMetaType::QRectF,
        QMetaType::QQuaternion,
        QMetaType::QMatrix4x4
};

template<>
struct ShaderType<QMetaType::Double>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Float; }
    static QByteArray name() { return QByteArrayLiteral("float"); }
};

template<>
struct ShaderType<QMetaType::Bool>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Boolean; }
    static QByteArray name() { return QByteArrayLiteral("bool"); }
};

template<>
struct ShaderType<QMetaType::Int>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Integer; }
    static QByteArray name() { return QByteArrayLiteral("int"); }
};

template<>
struct ShaderType<QMetaType::QVector2D>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Vec2; }
    static QByteArray name() { return QByteArrayLiteral("vec2"); }
};

template<>
struct ShaderType<QMetaType::QVector3D>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Vec3; }
    static QByteArray name() { return QByteArrayLiteral("vec3"); }
};

template<>
struct ShaderType<QMetaType::QVector4D>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Vec4; }
    static QByteArray name() { return QByteArrayLiteral("vec4"); }
};

template<>
struct ShaderType<QMetaType::QColor>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Rgba; }
    static QByteArray name() { return QByteArrayLiteral("vec4"); }
};

template<>
struct ShaderType<QMetaType::QSize>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Size; }
    static QByteArray name() { return QByteArrayLiteral("vec2"); }
};

template<>
struct ShaderType<QMetaType::QSizeF>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::SizeF; }
    static QByteArray name() { return QByteArrayLiteral("vec2"); }
};

template<>
struct ShaderType<QMetaType::QPoint>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Point; }
    static QByteArray name() { return QByteArrayLiteral("vec2"); }
};

template<>
struct ShaderType<QMetaType::QPointF>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::PointF; }
    static QByteArray name() { return QByteArrayLiteral("vec2"); }
};

template<>
struct ShaderType<QMetaType::QRect>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Rect; }
    static QByteArray name() { return QByteArrayLiteral("vec4"); }
};

template<>
struct ShaderType<QMetaType::QRectF>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::RectF; }
    static QByteArray name() { return QByteArrayLiteral("vec4"); }
};

template<>
struct ShaderType<QMetaType::QQuaternion>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Quaternion; }
    static QByteArray name() { return QByteArrayLiteral("vec4"); }
};

template<>
struct ShaderType<QMetaType::QMatrix4x4>
{
    static constexpr QSSGRenderShaderValue::Type type() { return QSSGRenderShaderValue::Matrix4x4; }
    static QByteArray name() { return QByteArrayLiteral("mat4"); }
};

QByteArray uniformTypeName(QMetaType type)
{
    switch (type.id()) {
    case QMetaType::Double:
    case QMetaType::Float:
        return ShaderType<QMetaType::Double>::name();
    case QMetaType::Bool:
        return ShaderType<QMetaType::Bool>::name();
    case QMetaType::QVector2D:
        return ShaderType<QMetaType::QVector2D>::name();
    case QMetaType::QVector3D:
        return ShaderType<QMetaType::QVector3D>::name();
    case QMetaType::QVector4D:
        return ShaderType<QMetaType::QVector4D>::name();
    case QMetaType::Int:
        return ShaderType<QMetaType::Int>::name();
    case QMetaType::QColor:
        return ShaderType<QMetaType::QColor>::name();
    case QMetaType::QSize:
        return ShaderType<QMetaType::QSize>::name();
    case QMetaType::QSizeF:
        return ShaderType<QMetaType::QSizeF>::name();
    case QMetaType::QPoint:
        return ShaderType<QMetaType::QPoint>::name();
    case QMetaType::QPointF:
        return ShaderType<QMetaType::QPointF>::name();
    case QMetaType::QRect:
        return ShaderType<QMetaType::QRect>::name();
    case QMetaType::QRectF:
        return ShaderType<QMetaType::QRectF>::name();
    case QMetaType::QQuaternion:
        return ShaderType<QMetaType::QQuaternion>::name();
    case QMetaType::QMatrix4x4:
        return ShaderType<QMetaType::QMatrix4x4>::name();
    default:
        return QByteArray();
    }
}

QByteArray uniformTypeName(QSSGRenderShaderValue::Type type)
{
    switch (type) {
    case QSSGRenderShaderValue::Float:
        return ShaderType<QMetaType::Double>::name();
    case QSSGRenderShaderValue::Boolean:
        return ShaderType<QMetaType::Bool>::name();
    case QSSGRenderShaderValue::Integer:
        return ShaderType<QMetaType::Int>::name();
    case QSSGRenderShaderValue::Vec2:
        return ShaderType<QMetaType::QVector2D>::name();
    case QSSGRenderShaderValue::Vec3:
        return ShaderType<QMetaType::QVector3D>::name();
    case QSSGRenderShaderValue::Vec4:
        return ShaderType<QMetaType::QVector4D>::name();
    case QSSGRenderShaderValue::Rgba:
        return ShaderType<QMetaType::QColor>::name();
    case QSSGRenderShaderValue::Size:
        return ShaderType<QMetaType::QSize>::name();
    case QSSGRenderShaderValue::SizeF:
        return ShaderType<QMetaType::QSizeF>::name();
    case QSSGRenderShaderValue::Point:
        return ShaderType<QMetaType::QPoint>::name();
    case QSSGRenderShaderValue::PointF:
        return ShaderType<QMetaType::QPointF>::name();
    case QSSGRenderShaderValue::Rect:
        return ShaderType<QMetaType::QRect>::name();
    case QSSGRenderShaderValue::RectF:
        return ShaderType<QMetaType::QRectF>::name();
    case QSSGRenderShaderValue::Quaternion:
        return ShaderType<QMetaType::QQuaternion>::name();
    case QSSGRenderShaderValue::Matrix4x4:
        return ShaderType<QMetaType::QMatrix4x4>::name();
    default:
        return QByteArray();
    }
}

QSSGRenderShaderValue::Type uniformType(QMetaType type)
{
    switch (type.id()) {
    case QMetaType::Double:
    case QMetaType::Float:
        return ShaderType<QMetaType::Double>::type();
    case QMetaType::Bool:
        return ShaderType<QMetaType::Bool>::type();
    case QMetaType::QVector2D:
        return ShaderType<QMetaType::QVector2D>::type();
    case QMetaType::QVector3D:
        return ShaderType<QMetaType::QVector3D>::type();
    case QMetaType::QVector4D:
        return ShaderType<QMetaType::QVector4D>::type();
    case QMetaType::Int:
        return ShaderType<QMetaType::Int>::type();
    case QMetaType::QColor:
        return ShaderType<QMetaType::QColor>::type();
    case QMetaType::QSize:
        return ShaderType<QMetaType::QSize>::type();
    case QMetaType::QSizeF:
        return ShaderType<QMetaType::QSizeF>::type();
    case QMetaType::QPoint:
        return ShaderType<QMetaType::QPoint>::type();
    case QMetaType::QPointF:
        return ShaderType<QMetaType::QPointF>::type();
    case QMetaType::QRect:
        return ShaderType<QMetaType::QRect>::type();
    case QMetaType::QRectF:
        return ShaderType<QMetaType::QRectF>::type();
    case QMetaType::QQuaternion:
        return ShaderType<QMetaType::QQuaternion>::type();
    case QMetaType::QMatrix4x4:
        return ShaderType<QMetaType::QMatrix4x4>::type();
    default:
        return QSSGRenderShaderValue::Unknown;
    }
}

MetaTypeList supportedMetatypes()
{
    return {std::begin(qssg_metatype_list), std::end(qssg_metatype_list)};
}

}

QQuick3DShaderUtilsBuffer::TextureFormat QQuick3DShaderUtilsBuffer::mapRenderTextureFormat(QSSGRenderTextureFormat::Format fmt)
{
    using TextureFormat = QQuick3DShaderUtilsBuffer::TextureFormat;
    switch (fmt) {
    case QSSGRenderTextureFormat::RGBA8: return TextureFormat::RGBA8;
    case QSSGRenderTextureFormat::RGBA16F: return TextureFormat::RGBA16F;
    case QSSGRenderTextureFormat::RGBA32F: return TextureFormat::RGBA32F;
    case QSSGRenderTextureFormat::R8: return TextureFormat::R8;
    case QSSGRenderTextureFormat::R16: return TextureFormat::R16;
    case QSSGRenderTextureFormat::R16F: return TextureFormat::R16F;
    case QSSGRenderTextureFormat::R32F: return TextureFormat::R32F;
    default:
        break;
    }
    return TextureFormat::Unknown;
}

QSSGRenderTextureFormat::Format QQuick3DShaderUtilsBuffer::mapTextureFormat(QQuick3DShaderUtilsBuffer::TextureFormat fmt)
{
    using TextureFormat = QQuick3DShaderUtilsBuffer::TextureFormat;
    switch (fmt) {
    case TextureFormat::RGBA8: return QSSGRenderTextureFormat::RGBA8;
    case TextureFormat::RGBA16F: return QSSGRenderTextureFormat::RGBA16F;
    case TextureFormat::RGBA32F: return QSSGRenderTextureFormat::RGBA32F;
    case TextureFormat::R8: return QSSGRenderTextureFormat::R8;
    case TextureFormat::R16: return QSSGRenderTextureFormat::R16;
    case TextureFormat::R16F: return QSSGRenderTextureFormat::R16F;
    case TextureFormat::R32F: return QSSGRenderTextureFormat::R32F;
    default:
        break;
    }
    return QSSGRenderTextureFormat::Unknown;
}

QQuick3DShaderUtilsBuffer::TextureFormat QQuick3DShaderUtilsBuffer::format() const
{
    return mapRenderTextureFormat(command.m_format.format);
}

void QQuick3DShaderUtilsBuffer::setFormat(TextureFormat format)
{
    QSSGRenderTextureFormat::Format mappedTextureFormat = mapTextureFormat(format);
    if (command.m_format == mappedTextureFormat)
        return;

    command.m_format = mappedTextureFormat;
    emit changed();
}

void QQuick3DShaderUtilsBuffer::setTextureFilterOperation(TextureFilterOperation op)
{
    if (command.m_filterOp == QSSGRenderTextureFilterOp(op))
        return;

    command.m_filterOp = QSSGRenderTextureFilterOp(op);
    emit changed();
}

void QQuick3DShaderUtilsBuffer::setTextureCoordOperation(TextureCoordOperation texCoordOp)
{
    if (command.m_texCoordOp == QSSGRenderTextureCoordOp(texCoordOp))
        return;

    command.m_texCoordOp = QSSGRenderTextureCoordOp(texCoordOp);
    emit changed();
}

void QQuick3DShaderUtilsBuffer::setBufferFlags(AllocateBufferFlagValues flag)
{
    if (quint32(command.m_bufferFlags) == quint32(flag))
        return;

    command.m_bufferFlags = quint32(flag);
    emit changed();
}

QQuick3DShaderUtilsRenderPass::~QQuick3DShaderUtilsRenderPass()
{

}

void QQuick3DShaderUtilsRenderPass::qmlAppendCommand(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list,
                                                     QQuick3DShaderUtilsRenderCommand *command)
{
    if (!command)
        return;

    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);
    that->m_commands.push_back(command);
    emit that->changed();
}

QQuick3DShaderUtilsRenderCommand *QQuick3DShaderUtilsRenderPass::qmlCommandAt(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list,
                                                                              qsizetype index)
{
    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);
    return that->m_commands.at(index);
}

qsizetype QQuick3DShaderUtilsRenderPass::qmlCommandCount(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list)
{
    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);
    return that->m_commands.size();
}

void QQuick3DShaderUtilsRenderPass::qmlCommandClear(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list)
{
    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);
    that->m_commands.clear();
    emit that->changed();
}

QQmlListProperty<QQuick3DShaderUtilsRenderCommand> QQuick3DShaderUtilsRenderPass::commands()
{
    return QQmlListProperty<QQuick3DShaderUtilsRenderCommand>(this,
                                                             nullptr,
                                                             QQuick3DShaderUtilsRenderPass::qmlAppendCommand,
                                                             QQuick3DShaderUtilsRenderPass::qmlCommandCount,
                                                             QQuick3DShaderUtilsRenderPass::qmlCommandAt,
                                                             QQuick3DShaderUtilsRenderPass::qmlCommandClear);
}

void QQuick3DShaderUtilsRenderPass::qmlAppendShader(QQmlListProperty<QQuick3DShaderUtilsShader> *list,
                                                    QQuick3DShaderUtilsShader *shader)
{
    if (!shader)
        return;

    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);

    // An append implementation CANNOT rely on the object (shader in this case)
    // being complete. When the list references a Shader object living under
    // another Effect, its properties may not be set at the point of this
    // function being called, so accessing shader->stage is not allowed since
    // it may still have its default value, not what is set from QML...

    // the only thing we can do is to append to our list, do not try to be clever
    that->m_shaders.append(shader);

    connect(shader, &QQuick3DShaderUtilsShader::shaderChanged, that, &QQuick3DShaderUtilsRenderPass::changed);
    connect(shader, &QQuick3DShaderUtilsShader::stageChanged, that, &QQuick3DShaderUtilsRenderPass::changed);

    emit that->changed();
}

QQuick3DShaderUtilsShader *QQuick3DShaderUtilsRenderPass::qmlShaderAt(QQmlListProperty<QQuick3DShaderUtilsShader> *list,
                                                                      qsizetype index)
{
    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);
    return that->m_shaders.at(index);
}

qsizetype QQuick3DShaderUtilsRenderPass::qmlShaderCount(QQmlListProperty<QQuick3DShaderUtilsShader> *list)
{
    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);
    return that->m_shaders.size();
}

void QQuick3DShaderUtilsRenderPass::qmlShaderClear(QQmlListProperty<QQuick3DShaderUtilsShader> *list)
{
    QQuick3DShaderUtilsRenderPass *that = qobject_cast<QQuick3DShaderUtilsRenderPass *>(list->object);

    for (QQuick3DShaderUtilsShader *shader : that->m_shaders)
        shader->disconnect(that);

    that->m_shaders.clear();

    emit that->changed();
}

QQmlListProperty<QQuick3DShaderUtilsShader> QQuick3DShaderUtilsRenderPass::shaders()
{
    return QQmlListProperty<QQuick3DShaderUtilsShader>(this,
                                                      nullptr,
                                                      QQuick3DShaderUtilsRenderPass::qmlAppendShader,
                                                      QQuick3DShaderUtilsRenderPass::qmlShaderCount,
                                                      QQuick3DShaderUtilsRenderPass::qmlShaderAt,
                                                      QQuick3DShaderUtilsRenderPass::qmlShaderClear);
}

QQuick3DShaderUtilsTextureInput::QQuick3DShaderUtilsTextureInput(QQuick3DObject *p) : QQuick3DObject(p) {}

QQuick3DShaderUtilsTextureInput::~QQuick3DShaderUtilsTextureInput()
{
}

void QQuick3DShaderUtilsTextureInput::setTexture(QQuick3DTexture *texture)
{
    if (m_texture == texture)
        return;

    QObject *p = parent();
    while (p != nullptr) {
        if (QQuick3DCustomMaterial *mat = qobject_cast<QQuick3DCustomMaterial *>(p)) {
            mat->setDynamicTextureMap(this);
            QQuick3DObjectPrivate::attachWatcherPriv(mat, this, &QQuick3DShaderUtilsTextureInput::setTexture, texture, m_texture);
            break;
        } else if (QQuick3DEffect *efx = qobject_cast<QQuick3DEffect *>(p)) {
            efx->setDynamicTextureMap(this);
            QQuick3DObjectPrivate::attachWatcherPriv(efx, this, &QQuick3DShaderUtilsTextureInput::setTexture, texture, m_texture);
            break;
        }
        p = p->parent();
    }

    if (p == nullptr) {
        qWarning("A TextureInput was defined without a CustomMaterial or Effect ancestor. This should be avoided.");
    }

    m_texture = texture;
    Q_EMIT textureChanged();
}

/*!
    \qmltype RenderablesFilter
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Defines a filter for selecting which renderables to affect in a \l {RenderPass}{pass}.
    \since 6.11

    The RenderablesFilter type is used to specify which renderables in the scene
    should be affected by a \l RenderPass. By setting the \c renderableTypes property,
    you can control whether the pass affects opaque objects, transparent objects, or
    no objects at all.

    Setting \c renderableTypes to \c None is useful when a RenderPass acts as a container
    for \l SubRenderPass commands and should not render any scene objects itself.

    In addition to filtering by renderable types, you can use the \l {RenderablesFilter::layerMask}{layerMask}
    to further refine which renderables are affected based on their assigned \l{Node::layers}{layers}.

    \qml
    // A pass that renders only opaque scene objects
    RenderPass {
        id: opaquePass
        commands: [
            ColorAttachment { name: "color0"; target: opaqueColorTexture },
            DepthTextureAttachment { target: depthTexture },
            RenderablesFilter {
                renderableTypes: RenderablesFilter.Opaque
            }
        ]
    }

    // A pass that renders only transparent objects on top
    RenderPass {
        id: transparentPass
        commands: [
            ColorAttachment { name: "color0"; target: transparentColorTexture },
            RenderablesFilter {
                renderableTypes: RenderablesFilter.Transparent
            }
        ]
    }
    \endqml
*/

/*!
    \qmlproperty int RenderablesFilter::layerMask
    Sets the layer mask for the filter. Only renderables on the specified layers will be affected by the filter.

    \sa Node::layers
*/

/*!
    \qmlproperty enumeration RenderablesFilter::renderableTypes
    Sets the types of renderables that the filter will affect.

    \value RenderablesFilter.None No renderables will be rendered. Useful for container passes that only have SubRenderPasses.
    \value RenderablesFilter.Opaque Only opaque renderables will be rendered.
    \value RenderablesFilter.Transparent Only transparent renderables will be rendered.

    \note Multiple values can be combined using the | operator.

    \default RenderablesFilter.Opaque | RenderablesFilter.Transparent
*/
QQuick3DShaderUtilsRenderablesFilter::RenderableTypes QQuick3DShaderUtilsRenderablesFilter::renderableTypes() const
{
    return static_cast<QQuick3DShaderUtilsRenderablesFilter::RenderableTypes>(command.renderableTypes);
}

QQuick3DShaderUtilsRenderablesFilter::~QQuick3DShaderUtilsRenderablesFilter()
{

}

void QQuick3DShaderUtilsRenderablesFilter::setRenderableTypes(RenderableTypes types)
{
    command.renderableTypes = static_cast<QSSGRenderablesFilterCommand::RenderableTypeT>(types.toInt());
}

QQuick3DShaderUtilsPipelineStateOverride::~QQuick3DShaderUtilsPipelineStateOverride()
{

}

/*!
    \qmlproperty bool PipelineStateOverride::depthTestEnabled
    If set to \c true, enables depth testing for the render pass. If set to \c false, disables depth testing.
    Setting this property to \c true requires a depth attachment for the render pass.
*/
bool QQuick3DShaderUtilsPipelineStateOverride::depthTestEnabled() const
{
    if (command.m_depthTestEnabled)
        return *command.m_depthTestEnabled;
    return false;
}

void QQuick3DShaderUtilsPipelineStateOverride::setDepthTestEnabled(bool newDepthTestEnabled)
{
    if (command.m_depthTestEnabled && *command.m_depthTestEnabled == newDepthTestEnabled)
        return;
    command.m_depthTestEnabled = newDepthTestEnabled;
    emit depthTestEnabledChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetDepthTestEnabled()
{
    command.m_depthTestEnabled.reset();
    emit depthTestEnabledChanged();
}

/*!
    \qmlproperty bool PipelineStateOverride::depthWriteEnabled
    If set to \c true, enables depth writing for the render pass. If set to \c false, disables depth writing.
    Setting this property to \c true requires a depth attachment for the render pass.
*/
bool QQuick3DShaderUtilsPipelineStateOverride::depthWriteEnabled() const
{
    if (command.m_depthWriteEnabled)
        return *command.m_depthWriteEnabled;
    return false;
}

void QQuick3DShaderUtilsPipelineStateOverride::setDepthWriteEnabled(bool newDepthWriteEnabled)
{
    if (command.m_depthWriteEnabled && *command.m_depthWriteEnabled == newDepthWriteEnabled)
        return;
    command.m_depthWriteEnabled = newDepthWriteEnabled;
    emit depthWriteEnabledChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetDepthWriteEnabled()
{
    command.m_depthWriteEnabled.reset();
    emit depthWriteEnabledChanged();
}

/*!
    \qmlproperty bool PipelineStateOverride::blendEnabled
    If set to \c true, enables blending for the render pass. If set to \c false, disables blending.
    For per-attachment blend settings, use the \c targetBlend0 through \c targetBlend7 properties.
*/
bool QQuick3DShaderUtilsPipelineStateOverride::blendEnabled() const
{
    if (command.m_blendEnabled)
        return *command.m_blendEnabled;
    return false;
}

void QQuick3DShaderUtilsPipelineStateOverride::setBlendEnabled(bool newBlendEnabled)
{
    if (command.m_blendEnabled && *command.m_blendEnabled == newBlendEnabled)
        return;
    command.m_blendEnabled = newBlendEnabled;
    emit blendEnabledChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetBlendEnabled()
{
    command.m_blendEnabled.reset();
    emit blendEnabledChanged();
}

/*!
    \qmlproperty bool PipelineStateOverride::usesStencilReference
    If set to \c true, enables the use of the stencil reference value for the render pass. If set to \c false,
    disables the use of the stencil reference value.
*/
bool QQuick3DShaderUtilsPipelineStateOverride::usesStencilReference() const
{
    if (command.m_usesStencilReference)
        return *command.m_usesStencilReference;
    return false;
}

void QQuick3DShaderUtilsPipelineStateOverride::setUsesStencilReference(bool newUsesStencilReference)
{
    if (command.m_usesStencilReference && *command.m_usesStencilReference == newUsesStencilReference)
        return;
    command.m_usesStencilReference = newUsesStencilReference;
    emit usesStencilReferenceChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetUsesStencilReference()
{
    command.m_usesStencilReference.reset();
    emit usesStencilReferenceChanged();
}

/*!
    \qmlproperty bool PipelineStateOverride::usesScissor
    If set to \c true, enables scissor testing for the render pass. If set to \c false,
    disables the scissor test. Use the \c scissor property to set the scissor rectangle.
*/
bool QQuick3DShaderUtilsPipelineStateOverride::usesScissor() const
{
    if (command.m_usesScissor)
        return *command.m_usesScissor;
    return false;
}

void QQuick3DShaderUtilsPipelineStateOverride::setUsesScissor(bool newUsesScissor)
{
    if (command.m_usesScissor && *command.m_usesScissor == newUsesScissor)
        return;
    command.m_usesScissor = newUsesScissor;
    emit usesScissorChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetUsesScissor()
{
    command.m_usesScissor.reset();
    emit usesScissorChanged();
}

/*!
    \qmlproperty enumeration PipelineStateOverride::depthFunction
    Sets the depth comparison function for the render pass.

    \value PipelineStateOverride.Never The depth test never passes.
    \value PipelineStateOverride.Less The depth test passes when the incoming depth is less than the stored depth.
    \value PipelineStateOverride.Equal The depth test passes when the incoming depth equals the stored depth.
    \value PipelineStateOverride.LessOrEqual The depth test passes when the incoming depth is less than or equal to the stored depth.
    \value PipelineStateOverride.Greater The depth test passes when the incoming depth is greater than the stored depth.
    \value PipelineStateOverride.NotEqual The depth test passes when the incoming depth does not equal the stored depth.
    \value PipelineStateOverride.GreaterOrEqual The depth test passes when the incoming depth is greater than or equal to the stored depth.
    \value PipelineStateOverride.Always The depth test always passes.
*/
QQuick3DShaderUtilsPipelineStateOverride::CompareOperation QQuick3DShaderUtilsPipelineStateOverride::depthFunction() const
{
    if (command.m_depthFunction)
        return CompareOperation(*command.m_depthFunction);
    return CompareOperation::Less;
}

void QQuick3DShaderUtilsPipelineStateOverride::setDepthFunction(CompareOperation newDepthFunction)
{
    if (command.m_depthFunction && *command.m_depthFunction == QRhiGraphicsPipeline::CompareOp(newDepthFunction))
        return;
    command.m_depthFunction = QRhiGraphicsPipeline::CompareOp(newDepthFunction);
    emit depthFunctionChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetDepthFunction()
{
    command.m_depthFunction.reset();
    emit depthFunctionChanged();
}

/*!
    \qmlproperty enumeration PipelineStateOverride::cullMode
    Sets the face culling mode for the render pass.

    \value PipelineStateOverride.None No face culling.
    \value PipelineStateOverride.Front Front-facing polygons are culled.
    \value PipelineStateOverride.Back Back-facing polygons are culled.
*/
QQuick3DShaderUtilsPipelineStateOverride::CullMode QQuick3DShaderUtilsPipelineStateOverride::cullMode() const
{
    if (command.m_cullMode)
        return CullMode(*command.m_cullMode);
    return CullMode::Back;
}

void QQuick3DShaderUtilsPipelineStateOverride::setCullMode(CullMode newCullMode)
{
    if (command.m_cullMode && *command.m_cullMode == QRhiGraphicsPipeline::CullMode(newCullMode))
        return;
    command.m_cullMode = QRhiGraphicsPipeline::CullMode(newCullMode);
    emit cullModeChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetCullMode()
{
    command.m_cullMode.reset();
    emit cullModeChanged();
}

/*!
    \qmlproperty enumeration PipelineStateOverride::polygonMode
    Sets the polygon rasterization mode for the render pass.

    \value PipelineStateOverride.Fill Polygons are filled (default).
    \value PipelineStateOverride.Line Polygon edges are drawn as lines (wireframe).
*/
QQuick3DShaderUtilsPipelineStateOverride::PolygonMode QQuick3DShaderUtilsPipelineStateOverride::polygonMode() const
{
    if (command.m_polygonMode)
        return PolygonMode(*command.m_polygonMode);
    return PolygonMode::Fill;
}

void QQuick3DShaderUtilsPipelineStateOverride::setPolygonMode(PolygonMode newPolygonMode)
{
    if (command.m_polygonMode && *command.m_polygonMode == QRhiGraphicsPipeline::PolygonMode(newPolygonMode))
        return;
    command.m_polygonMode = QRhiGraphicsPipeline::PolygonMode(newPolygonMode);
    emit polygonModeChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetPolygonMode()
{
    command.m_polygonMode.reset();
    emit polygonModeChanged();
}

/*!
    \qmlproperty uint PipelineStateOverride::stencilWriteMask
    Sets the stencil write mask for the render pass. Each bit controls whether the corresponding
    bit in the stencil buffer can be written.
*/
quint32 QQuick3DShaderUtilsPipelineStateOverride::stencilWriteMask() const
{
    if (command.m_stencilWriteMask)
        return *command.m_stencilWriteMask;
    return 0;
}

void QQuick3DShaderUtilsPipelineStateOverride::setStencilWriteMask(quint32 newStencilWriteMask)
{
    if (command.m_stencilWriteMask && *command.m_stencilWriteMask == newStencilWriteMask)
        return;
    command.m_stencilWriteMask = newStencilWriteMask;
    emit stencilWriteMaskChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetStencilWriteMask()
{
    command.m_stencilWriteMask.reset();
    emit stencilWriteMaskChanged();
}

/*!
    \qmlproperty uint PipelineStateOverride::stencilReference
    Sets the stencil reference value for the render pass. This value is used in stencil
    comparison operations when \c usesStencilReference is \c true.
*/
quint32 QQuick3DShaderUtilsPipelineStateOverride::stencilReference() const
{
    if (command.m_stencilReference)
        return *command.m_stencilReference;
    return 0;
}

void QQuick3DShaderUtilsPipelineStateOverride::setStencilReference(quint32 newStencilReference)
{
    if (command.m_stencilReference && *command.m_stencilReference == newStencilReference)
        return;
    command.m_stencilReference = newStencilReference;
    emit stencilReferenceChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetStencilReference()
{
    command.m_stencilReference.reset();
    emit stencilReferenceChanged();
}

/*!
    \qmlproperty rect PipelineStateOverride::viewport
    Sets the viewport rectangle for the render pass. The rectangle specifies the region
    of the render target to draw into, in pixels (x, y, width, height).
    If not set, the full render target dimensions are used.
*/
QRectF QQuick3DShaderUtilsPipelineStateOverride::viewport() const
{
    if (command.m_viewport) {
        const QRhiViewport &vp = *command.m_viewport;
        return QRectF(vp.viewport()[0], vp.viewport()[1], vp.viewport()[2], vp.viewport()[3]);
    }
    return QRectF();
}

void QQuick3DShaderUtilsPipelineStateOverride::setViewport(const QRectF &newViewport)
{
    if (command.m_viewport) {
        const QRhiViewport &vp = *command.m_viewport;
        if (vp.viewport()[0] == newViewport.x() &&
            vp.viewport()[1] == newViewport.y() &&
            vp.viewport()[2] == newViewport.width() &&
            vp.viewport()[3] == newViewport.height())
        return;
    }
    command.m_viewport = QRhiViewport(newViewport.x(), newViewport.y(), newViewport.width(), newViewport.height());
    emit viewportChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetViewport()
{
    command.m_viewport.reset();
    emit viewportChanged();
}

/*!
    \qmlproperty rect PipelineStateOverride::scissor
    Sets the scissor rectangle for the render pass. Fragments outside this rectangle are
    discarded. Requires \c usesScissor to be \c true.
*/
QRect QQuick3DShaderUtilsPipelineStateOverride::scissor() const
{
    if (command.m_scissor) {
        const QRhiScissor &sc = *command.m_scissor;
        return QRect(sc.scissor()[0], sc.scissor()[1], sc.scissor()[2], sc.scissor()[3]);
    }
    return QRect();
}

void QQuick3DShaderUtilsPipelineStateOverride::setScissor(const QRect &newScissor)
{
    if (command.m_viewport) {
        const QRhiScissor &sc = *command.m_scissor;
        if (sc.scissor()[0] == newScissor.x() &&
            sc.scissor()[1] == newScissor.y() &&
            sc.scissor()[2] == newScissor.width() &&
            sc.scissor()[3] == newScissor.height())
            return;
    }
    command.m_scissor = QRhiScissor(newScissor.x(), newScissor.y(), newScissor.width(), newScissor.height());
    emit scissorChanged();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetScissor()
{
    command.m_scissor.reset();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend0
    Sets the blending parameters for color attachment 0 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend0() const
{
    if (command.m_targetBlend0)
        return *command.m_targetBlend0;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend0(const QQuick3DRenderPassTargetBlend &newTargetBlend0)
{
    if (command.m_targetBlend0 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend0) == newTargetBlend0)
        return;

    command.m_targetBlend0 = newTargetBlend0.toRhiTargetBlend();;
    emit targetBlend0Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend0()
{
    command.m_targetBlend0.reset();
    emit targetBlend0Changed();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend1
    Sets the blending parameters for color attachment 1 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend1() const
{
    if (command.m_targetBlend1)
        return *command.m_targetBlend1;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend1(const QQuick3DRenderPassTargetBlend &newTargetBlend1)
{
    if (command.m_targetBlend1 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend1) == newTargetBlend1)
        return;
    command.m_targetBlend1 = newTargetBlend1.toRhiTargetBlend();
    emit targetBlend1Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend1()
{
    command.m_targetBlend1.reset();
    emit targetBlend1Changed();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend2
    Sets the blending parameters for color attachment 2 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend2() const
{
    if (command.m_targetBlend2)
        return *command.m_targetBlend2;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend2(const QQuick3DRenderPassTargetBlend &newTargetBlend2)
{
    if (command.m_targetBlend2 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend2) == newTargetBlend2)
        return;
    command.m_targetBlend2 = newTargetBlend2.toRhiTargetBlend();
    emit targetBlend2Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend2()
{
    command.m_targetBlend2.reset();
    emit targetBlend2Changed();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend3
    Sets the blending parameters for color attachment 3 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend3() const
{
    if (command.m_targetBlend3)
        return *command.m_targetBlend3;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend3(const QQuick3DRenderPassTargetBlend &newTargetBlend3)
{
    if (command.m_targetBlend3 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend3) == newTargetBlend3)
        return;
    command.m_targetBlend3 = newTargetBlend3.toRhiTargetBlend();
    emit targetBlend3Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend3()
{
    command.m_targetBlend3.reset();
    emit targetBlend3Changed();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend4
    Sets the blending parameters for color attachment 4 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend4() const
{
    if (command.m_targetBlend4)
        return *command.m_targetBlend4;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend4(const QQuick3DRenderPassTargetBlend &newTargetBlend4)
{
    if (command.m_targetBlend4 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend4) == newTargetBlend4)
        return;
    command.m_targetBlend4 = newTargetBlend4.toRhiTargetBlend();
    emit targetBlend4Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend4()
{
    command.m_targetBlend4.reset();
    emit targetBlend4Changed();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend5
    Sets the blending parameters for color attachment 5 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend5() const
{
    if (command.m_targetBlend5)
        return *command.m_targetBlend5;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend5(const QQuick3DRenderPassTargetBlend &newTargetBlend5)
{
    if (command.m_targetBlend5 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend5) == newTargetBlend5)
        return;
    command.m_targetBlend5 = newTargetBlend5.toRhiTargetBlend();
    emit targetBlend5Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend5()
{
    command.m_targetBlend5.reset();
    emit targetBlend5Changed();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend6
    Sets the blending parameters for color attachment 6 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend6() const
{
    if (command.m_targetBlend6)
        return *command.m_targetBlend6;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend6(const QQuick3DRenderPassTargetBlend &newTargetBlend6)
{
    if (command.m_targetBlend6 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend6) == newTargetBlend6)
        return;
    command.m_targetBlend6 = newTargetBlend6.toRhiTargetBlend();
    emit targetBlend6Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend6()
{
    command.m_targetBlend6.reset();
    emit targetBlend6Changed();
}

/*!
    \qmlproperty renderTargetBlend PipelineStateOverride::targetBlend7
    Sets the blending parameters for color attachment 7 of the render pass.
    \sa renderTargetBlend
*/
QQuick3DRenderPassTargetBlend QQuick3DShaderUtilsPipelineStateOverride::targetBlend7() const
{
    if (command.m_targetBlend7)
        return *command.m_targetBlend7;
    return {};
}

void QQuick3DShaderUtilsPipelineStateOverride::setTargetBlend7(const QQuick3DRenderPassTargetBlend &newTargetBlend7)
{
    if (command.m_targetBlend7 && QQuick3DRenderPassTargetBlend(*command.m_targetBlend7) == newTargetBlend7)
        return;
    command.m_targetBlend7 = newTargetBlend7.toRhiTargetBlend();
    emit targetBlend7Changed();
}

void QQuick3DShaderUtilsPipelineStateOverride::resetTargetBlend7()
{
    command.m_targetBlend7.reset();
    emit targetBlend7Changed();
}

/*!
    \qmltype RenderPassTexture
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \brief Defines a texture to be used as a render target in a \l {RenderPass}{pass}.
    \since 6.11

    RenderPassTexture declares an off-screen texture that a \l RenderPass can
    render into. The texture is automatically created and managed by the
    rendering system. Its size tracks the \l View3D viewport by default.

    Once declared, attach the texture to a pass via \l ColorAttachment (for
    color output) or \l DepthTextureAttachment (for depth output). The
    resulting texture can then be accessed from a subsequent pass or a
    \l Texture component using \l RenderOutputProvider.

    \qml
    // Declare a float color buffer
    RenderPassTexture {
        id: hdrColorTexture
        format: RenderPassTexture.RGBA16F
    }

    RenderPass {
        commands: [
            ColorAttachment {
                name: "color0"
                target: hdrColorTexture
            }
        ]
    }
    \endqml

    \sa ColorAttachment, DepthTextureAttachment, RenderOutputProvider
*/

/*!
    \qmlproperty enumeration RenderPassTexture::format
    Sets the format of the render target texture.

    Available formats are:

    \value RenderPassTexture.Unknown
    \value RenderPassTexture.RGBA8
    \value RenderPassTexture.RGBA16F
    \value RenderPassTexture.RGBA32F
    \value RenderPassTexture.R8
    \value RenderPassTexture.R16
    \value RenderPassTexture.R16F
    \value RenderPassTexture.R32F
    \value RenderPassTexture.Depth16
    \value RenderPassTexture.Depth24
    \value RenderPassTexture.Depth32
    \value RenderPassTexture.Depth24Stencil8
*/

QQuick3DShaderUtilsRenderPassTexture::TextureFormat QQuick3DShaderUtilsRenderPassTexture::format() const
{
    return fromRenderTextureFormat(command->format());
}

QQuick3DShaderUtilsRenderPassTexture::~QQuick3DShaderUtilsRenderPassTexture()
{

}

void QQuick3DShaderUtilsRenderPassTexture::setFormat(TextureFormat newFormat)
{
    if (!command)
        command = std::make_shared<QSSGAllocateTexture>();
    command->setFormat(asRenderTextureFormat(newFormat));
}

QSSGRenderTextureFormat QQuick3DShaderUtilsRenderPassTexture::asRenderTextureFormat(TextureFormat fmt)
{
    switch (fmt) {
    case TextureFormat::Unknown: return QSSGRenderTextureFormat::Unknown;
    case TextureFormat::RGBA8: return QSSGRenderTextureFormat::RGBA8;
    case TextureFormat::RGBA16F: return QSSGRenderTextureFormat::RGBA16F;
    case TextureFormat::RGBA32F: return QSSGRenderTextureFormat::RGBA32F;
    case TextureFormat::R8: return QSSGRenderTextureFormat::R8;
    case TextureFormat::R16: return QSSGRenderTextureFormat::R16;
    case TextureFormat::R16F: return QSSGRenderTextureFormat::R16F;
    case TextureFormat::R32F: return QSSGRenderTextureFormat::R32F;
    case TextureFormat::Depth16: return QSSGRenderTextureFormat::Depth16;
    case TextureFormat::Depth24: return QSSGRenderTextureFormat::Depth24;
    case TextureFormat::Depth32: return QSSGRenderTextureFormat::Depth32;
    case TextureFormat::Depth24Stencil8: return QSSGRenderTextureFormat::Depth24Stencil8;
    default:
        break;
    }
    return QSSGRenderTextureFormat::Unknown;
}

QQuick3DShaderUtilsRenderPassTexture::TextureFormat QQuick3DShaderUtilsRenderPassTexture::fromRenderTextureFormat(QSSGRenderTextureFormat fmt)
{
    switch (fmt.format) {
    case QSSGRenderTextureFormat::Unknown: return TextureFormat::Unknown;
    case QSSGRenderTextureFormat::RGBA8: return TextureFormat::RGBA8;
    case QSSGRenderTextureFormat::RGBA16F: return TextureFormat::RGBA16F;
    case QSSGRenderTextureFormat::RGBA32F: return TextureFormat::RGBA32F;
    case QSSGRenderTextureFormat::R8: return TextureFormat::R8;
    case QSSGRenderTextureFormat::R16: return TextureFormat::R16;
    case QSSGRenderTextureFormat::R16F: return TextureFormat::R16F;
    case QSSGRenderTextureFormat::R32F: return TextureFormat::R32F;
    case QSSGRenderTextureFormat::Depth16: return TextureFormat::Depth16;
    case QSSGRenderTextureFormat::Depth24: return TextureFormat::Depth24;
    case QSSGRenderTextureFormat::Depth32: return TextureFormat::Depth32;
    case QSSGRenderTextureFormat::Depth24Stencil8: return TextureFormat::Depth24Stencil8;
    default:
        break;
    }
    return TextureFormat::Unknown;
}

/*!
    \qmltype ColorAttachment
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Defines a color attachment for a \l {RenderPass}{pass}.
    \since 6.11

    The ColorAttachment type is used to specify a color attachment for a \l RenderPass.
    The \l name property is used to identify the attachment within the render pass.
    If the \l {RenderPass.AugmentMaterial}{AugmentMaterial} mode is used, the name will be
    exposed as an output with \c name in the fragment shader.

    \sa RenderPassTexture

    \qml
    RenderPass {
        // Define a render target texture
        RenderPassTexture {
            id: colorTexture
            format: RenderPassTexture.RGBA16F
        }

        commands: [
            // Define a color attachment using the texture
            ColorAttachment {
                name: "color0"
                target: colorTexture
            }
        ]
    }
    \endqml
*/

/*!
    \qmlproperty RenderPassTexture ColorAttachment::target
    The \l RenderPassTexture that will be used as the color attachment. The texture
    must have a color-compatible format (e.g. \c RGBA8, \c RGBA16F).
*/

/*!
    \qmlproperty string ColorAttachment::name
    The name used to identify this color attachment within the render pass. When
    \l {RenderPass::materialMode}{materialMode} is set to \c AugmentMaterial, this name
    is exposed as a fragment shader output variable, allowing custom shader code to
    write to this attachment.
*/

QQuick3DShaderUtilsRenderPassColorAttachment::~QQuick3DShaderUtilsRenderPassColorAttachment()
{

}

QByteArray QQuick3DShaderUtilsRenderPassColorAttachment::name() const
{
    return m_name;
}

void QQuick3DShaderUtilsRenderPassColorAttachment::setName(const QByteArray &newName)
{
    m_name = newName;
}

QSSGCommand *QQuick3DShaderUtilsRenderPassColorAttachment::cloneCommand()
{
    if (target) {
        QSSGColorAttachment *cmd = new QSSGColorAttachment(m_name);
        cmd->m_textureCmd = target->command;
        return cmd;
    }

    return nullptr;
}

QQuick3DPropertyChangedTracker::~QQuick3DPropertyChangedTracker()
{

}

void QQuick3DPropertyChangedTracker::extractProperties(UniformPropertyList &outUniforms)
{
    // Ensure we start with a clean list
    outUniforms.clear();

    auto metaObject = m_owner->metaObject();

    // Properties -> uniforms
    const int propCount = metaObject->propertyCount();
    int propOffset = metaObject->propertyOffset();

    // Classes can have multilayered inheritance structure, so find the actual propOffset by
    // walking up the inheritance chain.
    const QMetaObject *superClass = metaObject->superClass();
    while (superClass && qstrcmp(superClass->className(), m_superClassName) != 0)  {
        propOffset = superClass->propertyOffset();
        superClass = superClass->superClass();
    }

    static auto getSamplerHint = [](const QQuick3DTexture &texture) {
        if (auto *po = QQuick3DObjectPrivate::get(&texture)) {
            if (po->type == QQuick3DObjectPrivate::Type::TextureProvider) {
                auto textureProvider = static_cast<QQuick3DTextureProviderExtension *>(texture.textureProvider());
                switch (textureProvider->samplerHint()) {
                case QQuick3DTextureProviderExtension::SamplerHint::Sampler2D:
                    return QSSGRenderSamplerType::Sampler2D;
                case QQuick3DTextureProviderExtension::SamplerHint::Sampler2DArray:
                    return QSSGRenderSamplerType::Sampler2DArray;
                case QQuick3DTextureProviderExtension::SamplerHint::Sampler3D:
                    return QSSGRenderSamplerType::Sampler3D;
                case QQuick3DTextureProviderExtension::SamplerHint::SamplerCube:
                    return QSSGRenderSamplerType::SamplerCube;
                case QQuick3DTextureProviderExtension::SamplerHint::SamplerCubeArray:
                    return QSSGRenderSamplerType::SamplerCubeArray;
                case QQuick3DTextureProviderExtension::SamplerHint::SamplerBuffer:
                    return QSSGRenderSamplerType::SamplerBuffer;
                }
            } else if (po->type == QQuick3DObjectPrivate::Type::ImageCube) {
                return QSSGRenderSamplerType::SamplerCube;
            } else if (texture.textureData() && texture.textureData()->depth() > 0) {
                return QSSGRenderSamplerType::Sampler3D;
            }
        }

        return QSSGRenderSamplerType::Sampler2D;
    };

    const auto addTextureToUniforms = [&](const char *name, QQuick3DTexture *texture, int propertyIndex) {
        QSSGRenderImage *ri = static_cast<QSSGRenderImage *>(QQuick3DObjectPrivate::get(texture)->spatialNode);
        auto samplerName = QSSGBaseTypeHelpers::toString(getSamplerHint(*texture));
        outUniforms.emplace_back(name, samplerName, QVariant::fromValue(ri), QSSGRenderShaderValue::Texture, propertyIndex);
    };

    // The TextureInput type needs extra watchers for its properties...
    const auto addTextureInputWatchers = [&](QMetaProperty property, QQuick3DShaderUtilsTextureInput *textureInput) {
        QObject::connect(textureInput, &QQuick3DShaderUtilsTextureInput::enabledChanged, m_owner, [this, property, textureInput](){ addPropertyWatcher(property, DirtyPropertyHint::Reference, textureInput); }, Qt::UniqueConnection);
        QObject::connect(textureInput, &QQuick3DShaderUtilsTextureInput::textureChanged, m_owner, [this, property, textureInput](){ addPropertyWatcher(property, DirtyPropertyHint::Reference, textureInput); }, Qt::UniqueConnection);
        if (auto *texture = textureInput->texture())
            addTextureToUniforms(property.name(), texture, property.propertyIndex());
    };

    for (int i = propOffset; i != propCount; ++i) {
        const QMetaProperty property = metaObject->property(i);
        if (Q_UNLIKELY(!property.isValid()))
            continue;

        const char *name = property.name();
        QMetaType propType = property.metaType();
        QVariant propValue = property.read(m_owner);
        if (propType == QMetaType(QMetaType::QVariant))
            propType = propValue.metaType();

        const auto type = QSSGShaderUtils::uniformType(propType);
        if (type != QSSGRenderShaderValue::Unknown) {
            outUniforms.emplace_back(name, QSSGShaderUtils::uniformTypeName(propType),
                                     propValue, QSSGShaderUtils::uniformType(propType), i);
            addPropertyWatcher(property, DirtyPropertyHint::Value);
        } else {
            if (propType.id() >= QMetaType::User) {
                if (propType.id() == qMetaTypeId<QQuick3DTexture *>()) {
                    if (QQuick3DTexture *texture = property.read(m_owner).value<QQuick3DTexture *>()) {
                        addTextureToUniforms(name, texture, i);
                        addPropertyWatcher(property, DirtyPropertyHint::Reference, texture);
                    }
                } else if (propType.id() == qMetaTypeId<QQuick3DShaderUtilsTextureInput *>()) { // For compatibility, also check for texture input types
                    if (QQuick3DShaderUtilsTextureInput *textureInput = property.read(m_owner).value<QQuick3DShaderUtilsTextureInput *>(); textureInput && textureInput->texture()) {
                        addTextureInputWatchers(property, textureInput);
                        addPropertyWatcher(property, DirtyPropertyHint::Reference, textureInput);
                    }
                }
            } else if (propType == QMetaType(QMetaType::QObjectStar)) {
                if (QQuick3DTexture *texture = qobject_cast<QQuick3DTexture *>(propValue.value<QObject *>())) {
                    addTextureToUniforms(name, texture, i);
                    addPropertyWatcher(property, DirtyPropertyHint::Reference, texture);
                } else if (QQuick3DShaderUtilsTextureInput *textureInput = qobject_cast<QQuick3DShaderUtilsTextureInput *>(propValue.value<QObject *>()); textureInput && textureInput->texture()) {
                    addTextureInputWatchers(property, textureInput);
                    addPropertyWatcher(property, DirtyPropertyHint::Reference, textureInput);
                }
            }
        }
    }
}

void QQuick3DPropertyChangedTracker::addPropertyWatcher(QMetaProperty property, DirtyPropertyHint hint, QQuick3DObject *object)
{
    if (property.isValid() && property.hasNotifySignal()) {
        // Check if we're already watching this property.
        const auto pid = property.propertyIndex();
        Q_ASSERT(pid != -1);
        auto it = std::find_if(m_trackedProperties.begin(), m_trackedProperties.end(), [pid](const Tracked &tp) { return tp.pid == pid; });
        const bool found = (it != m_trackedProperties.end());
        QQuick3DObject *oldObj = nullptr;
        if (!found) {
            QQuick3DPropertyWatcher *watcher = new QQuick3DPropertyWatcher(this, property);
            m_trackedProperties.push_back({watcher, object, pid});
            it = std::prev(m_trackedProperties.end());
        } else {
            oldObj = it->object;
            it->object = object;
        }

        if (hint == DirtyPropertyHint::Reference) {
            const auto &sm = QQuick3DObjectPrivate::get(m_owner)->sceneManager;

            // First check if the object changed
            const bool changed = (oldObj != object);

            // Deref old object
            if (changed && oldObj) {
                QQuick3DObjectPrivate::get(oldObj)->derefSceneManager();
                QObject::disconnect(oldObj, &QObject::destroyed, m_owner, nullptr);
            }

            // Ref new object
            if (changed && object) {
                QQuick3DObjectPrivate::get(object)->refSceneManager(*sm);
                QObject::connect(object, &QObject::destroyed, m_owner, [this, property](QObject *obj) {
                    Q_UNUSED(obj);
                    addPropertyWatcher(property, DirtyPropertyHint::Reference, nullptr);
                });
            }
        }

        markTrackedPropertyDirty(property, hint);
    }
}

void QQuick3DPropertyChangedTracker::markTrackedPropertyDirty(QMetaProperty property, DirtyPropertyHint hint)
{
    Q_UNUSED(property);
    Q_UNUSED(hint);
    QSSG_CHECK_X(false, "QQuick3DPropertyChangedTracker::onPropertyDirty implementation missing");
}

QQuick3DPropertyWatcher::QQuick3DPropertyWatcher(QQuick3DPropertyChangedTracker *tracker, QMetaProperty property)
    : m_tracker(tracker)
    , m_property(property)
{
    Q_ASSERT(tracker != nullptr);
    Q_ASSERT(property.isValid() && property.hasNotifySignal());

    const bool isPointerType = (property.metaType().flags().testFlag(QMetaType::IsPointer));

    if (!isPointerType) {
        // Value change notification
        const auto idx = staticMetaObject.indexOfSlot("onValuePropertyChanged()");
        if (QSSG_GUARD_X(idx != -1, "Method not found!")) {
            auto onPropertyChangedMethod = staticMetaObject.method(idx);
            connect(m_tracker->m_owner, property.notifySignal(), this, onPropertyChangedMethod);
        }
    } else {
        // Pointer value change notification
        const auto idx = staticMetaObject.indexOfSlot("onPointerPropertyChanged()");
        if (QSSG_GUARD_X(idx != -1, "Method not found!")) {
            auto onPointerPropertyChangedMethod = staticMetaObject.method(idx);
            connect(m_tracker->m_owner, property.notifySignal(), this, onPointerPropertyChangedMethod);
        }
    }
}

void QQuick3DPropertyWatcher::onValuePropertyChanged()
{
    m_tracker->markTrackedPropertyDirty(m_property, QQuick3DPropertyChangedTracker::DirtyPropertyHint::Value);
}

void QQuick3DPropertyWatcher::onPointerPropertyChanged()
{
    m_tracker->markTrackedPropertyDirty(m_property, QQuick3DPropertyChangedTracker::DirtyPropertyHint::Reference);;
}

/*!
    \qmltype AddDefine
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Adds a preprocessor define to the shader compilation for a \l {RenderPass}{pass}.
    \since 6.11

    The AddDefine type is used to inject a \c {#define} into the shader
    compilation for a \l RenderPass. This lets you branch shader behavior
    at compile time depending on which pass is being executed.

    In the shader code, the define can be used with the standard preprocessor
    \c {#ifdef} / \c {#if} directives:
    \badcode
    #ifdef CUSTOM_PASS_MODE
    // code compiled only when this define is present
    #endif
    \endcode

    \qml
    RenderPass {
        commands: [
            ColorAttachment { name: "color0"; target: colorTexture },
            AddDefine { name: "CUSTOM_PASS_MODE" },
            AddDefine { name: "OUTPUT_VERSION"; value: 2 }
        ]
    }
    \endqml
*/

/*!
    \qmlproperty string AddDefine::name
    The name of the preprocessor macro to define. This becomes the identifier
    used in \c {#ifdef} or \c {#if} checks in the shader code.
*/

/*!
    \qmlproperty int AddDefine::value
    The integer value assigned to the preprocessor macro. Defaults to \c 1.
    When non-zero, \c {#if NAME} evaluates to \c true in the shader.
*/

QQuick3DShaderUtilsRenderPassAddDefine::QQuick3DShaderUtilsRenderPassAddDefine()
{

}

QQuick3DShaderUtilsRenderPassAddDefine::~QQuick3DShaderUtilsRenderPassAddDefine() = default;

QSSGCommand *QQuick3DShaderUtilsRenderPassAddDefine::cloneCommand() {
    QSSGAddShaderDefine *cmd = new QSSGAddShaderDefine(command);
    return cmd;
}

/*!
    \qmltype SubRenderPass
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Renders a sub-section of work within the same render target as the parent RenderPass.
    \since 6.11

    SubRenderPass is a \l Command that subdivides a \l RenderPass into
    smaller, independently configured pieces while sharing the same render
    target. Only the parent \l RenderPass controls which textures are
    rendered into (via \l ColorAttachment and \l DepthTextureAttachment)
    and when the pass begins (and therefore when the render target is
    cleared). A SubRenderPass cannot set its own render target or clear
    settings — those are always inherited from the parent pass.

    What a SubRenderPass \e can control is \e what gets rendered and
    \e how: it has its own \l{RenderPass::commands}{commands}, so it can
    carry a \l RenderablesFilter to select a subset of objects, a
    \l PipelineStateOverride to change depth/blend state, a different
    \l{RenderPass::materialMode}{materialMode}, and so on.

    A typical use is a forward-rendering pipeline where a single color
    pass uses separate SubRenderPasses for the background, opaque
    objects, and transparent objects. All three sub-passes write into the
    same color and depth textures, but each has different pipeline state:

    \qml
    RenderPass {
        id: mainColorPass
        commands: [
            // Render targets are set at the parent RenderPass level only
            ColorAttachment { name: "color0"; target: colorTexture },
            DepthTextureAttachment { target: depthTexture },

            // Sub-pass 1: render the skybox background
            SubRenderPass { renderPass: skyboxSubPass },

            // Sub-pass 2: opaque objects with depth testing and writing
            SubRenderPass { renderPass: opaqueSubPass },

            // Sub-pass 3: transparent objects with blending, no depth write
            SubRenderPass { renderPass: transparentSubPass }
        ]
    }

    // Sub-passes share mainColorPass's render target
    RenderPass {
        id: skyboxSubPass
        passMode: RenderPass.SkyboxPass
    }

    RenderPass {
        id: opaqueSubPass
        commands: [
            RenderablesFilter { renderableTypes: RenderablesFilter.Opaque }
        ]
    }

    RenderPass {
        id: transparentSubPass
        commands: [
            RenderablesFilter { renderableTypes: RenderablesFilter.Transparent },
            PipelineStateOverride {
                blendEnabled: true
                depthWriteEnabled: false
            }
        ]
    }
    \endqml

    \sa RenderPass, RenderablesFilter, PipelineStateOverride
*/

/*!
    \qmlproperty RenderPass SubRenderPass::renderPass
    The RenderPass that will be executed as a sub-pass when this command is processed.
*/

QQuick3DShaderUtilsSubRenderPass::~QQuick3DShaderUtilsSubRenderPass()
{

}

QSSGCommand *QQuick3DShaderUtilsSubRenderPass::cloneCommand()
{
    QSSGSubRenderPass *cmd = nullptr;

    if (!m_renderPass) {
        if (!m_hasWarnedAboutInvalidId) {
            qCWarning(lcSubRenderPass, "SubRenderPass: No render pass specified. Set the 'renderPass' property.");
            m_hasWarnedAboutInvalidId = true;
        }
        return nullptr;
    }

    QSSGResourceId userPassId = QQuick3DExtensionHelpers::getResourceId(*m_renderPass);
    // Ensure we have a valid resource id before continuing.
    if (userPassId != QSSGResourceId::Invalid) {
        cmd = new QSSGSubRenderPass();
        cmd->setSubPass(userPassId);
        // Reset warning flag on success
        m_hasWarnedAboutInvalidId = false;
    } else {
        // Resource ID is not ready yet - this is expected during initialization
        qCDebug(lcSubRenderPass, "SubRenderPass: Render pass resource ID not yet available, will retry.");
        update();
    }

    return cmd;
}

QSSGRenderGraphObject *QQuick3DShaderUtilsSubRenderPass::updateSpatialNode(QSSGRenderGraphObject *node)
{
    return node;
}

void QQuick3DShaderUtilsSubRenderPass::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

void QQuick3DShaderUtilsSubRenderPass::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    if (sceneManager)
        QQuick3DObjectPrivate::refSceneManager(m_renderPass, *sceneManager);
    else
        QQuick3DObjectPrivate::derefSceneManager(m_renderPass);
}

QQuick3DRenderPass *QQuick3DShaderUtilsSubRenderPass::renderPass() const
{
    return m_renderPass;
}

void QQuick3DShaderUtilsSubRenderPass::setRenderPass(QQuick3DRenderPass *newRenderPass)
{
    if (m_renderPass == newRenderPass)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DShaderUtilsSubRenderPass::setRenderPass, newRenderPass, m_renderPass);

    if (newRenderPass)
        newRenderPass->update();

    m_renderPass = newRenderPass;
    m_hasWarnedAboutInvalidId = false; // Reset warning flag when property changes
    emit renderPassChanged();
}

/*!
    \qmltype DepthTextureAttachment
    \inherits Command
    \inqmlmodule QtQuick3D
    \brief Defines a depth texture attachment for a \l {RenderPass}{pass}.
    \since 6.11

    DepthTextureAttachment attaches a \l RenderPassTexture with a depth
    format as the depth buffer for a \l RenderPass. Unlike
    \l DepthStencilAttachment (which uses an opaque render buffer), the
    resulting depth data is stored in a texture that can be read by
    subsequent passes via \l RenderOutputProvider.

    \qml
    RenderPassTexture {
        id: depthTexture
        format: RenderPassTexture.Depth24
    }

    RenderPass {
        commands: [
            ColorAttachment { name: "color0"; target: colorTexture },
            DepthTextureAttachment { target: depthTexture }
        ]
    }

    // Expose the depth texture to a material via RenderOutputProvider
    Texture {
        textureProvider: RenderOutputProvider {
            textureSource: RenderOutputProvider.UserPassTexture
            renderPass: myRenderPass
        }
    }
    \endqml

    \sa RenderPassTexture, DepthStencilAttachment, RenderOutputProvider
*/

/*!
    \qmlproperty RenderPassTexture DepthTextureAttachment::target
    The \l RenderPassTexture that will be used as the depth attachment. The texture
    must have a depth-compatible format (e.g. \c Depth16, \c Depth24, \c Depth32,
    or \c Depth24Stencil8).
*/

QQuick3DShaderUtilsRenderPassDepthTextureAttachment::~QQuick3DShaderUtilsRenderPassDepthTextureAttachment()
{

}

QSSGCommand *QQuick3DShaderUtilsRenderPassDepthTextureAttachment::cloneCommand()
{
    if (target) {
        QSSGDepthTextureAttachment *cmd = new QSSGDepthTextureAttachment(QByteArrayLiteral("__depth__"));
        cmd->m_textureCmd = target->command;
        return cmd;
    }

    return nullptr;
}

QQuick3DShaderUtilsRenderPassDepthStencilAttachment::~QQuick3DShaderUtilsRenderPassDepthStencilAttachment()
{

}

QSSGCommand *QQuick3DShaderUtilsRenderPassDepthStencilAttachment::cloneCommand()
{
    QSSGDepthStencilAttachment *cmd = new QSSGDepthStencilAttachment;
    return cmd;
}

QT_END_NAMESPACE


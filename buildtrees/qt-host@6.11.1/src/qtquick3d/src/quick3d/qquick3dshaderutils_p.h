// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DSHADERUTILS_H
#define QQUICK3DSHADERUTILS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3D/qtquick3dglobal.h>
#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3D/private/qquick3dtexture_p.h>
#include <QtQuick3D/private/qquick3dmaterial_p.h>

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendercommands_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DShaderUtilsShader;
class QQmlContext;
class QQuick3DRenderPass;

namespace QSSGShaderUtils {

using MetaTypeList = QList<QMetaType::Type>;
using ResolveFunction = bool (*)(const QUrl &url, const QQmlContext *context, QByteArray &shaderData, QByteArray &shaderPathKey);
Q_QUICK3D_EXPORT void setResolveFunction(ResolveFunction fn);
Q_QUICK3D_EXPORT QByteArray resolveShader(const QUrl &fileUrl, const QQmlContext *context, QByteArray &shaderPathKey);
Q_QUICK3D_EXPORT MetaTypeList supportedMetatypes();


template<QMetaType::Type>
struct ShaderType
{
};

Q_QUICK3D_EXPORT QByteArray uniformTypeName(QMetaType type);
Q_QUICK3D_EXPORT QByteArray uniformTypeName(QSSGRenderShaderValue::Type type);
Q_QUICK3D_EXPORT QSSGRenderShaderValue::Type uniformType(QMetaType type);
}

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsTextureInput : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DTexture *texture READ texture WRITE setTexture NOTIFY textureChanged)
    Q_PROPERTY(bool enabled MEMBER enabled NOTIFY enabledChanged)

    QML_NAMED_ELEMENT(TextureInput)

public:
    explicit QQuick3DShaderUtilsTextureInput(QQuick3DObject *p = nullptr);
    virtual ~QQuick3DShaderUtilsTextureInput();
    QQuick3DTexture *m_texture = nullptr;
    bool enabled = true;
    QByteArray name;
    QQuick3DTexture *texture() const
    {
        return m_texture;
    }

public Q_SLOTS:
    void setTexture(QQuick3DTexture *texture);

Q_SIGNALS:
    void textureChanged();
    void enabledChanged();
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsBuffer : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(TextureFormat format READ format WRITE setFormat NOTIFY changed)
    Q_PROPERTY(TextureFilterOperation textureFilterOperation READ textureFilterOperation WRITE setTextureFilterOperation NOTIFY changed)
    Q_PROPERTY(TextureCoordOperation textureCoordOperation READ textureCoordOperation WRITE setTextureCoordOperation NOTIFY changed)
    Q_PROPERTY(float sizeMultiplier MEMBER sizeMultiplier NOTIFY changed)
    Q_PROPERTY(AllocateBufferFlagValues bufferFlags READ bufferFlags WRITE setBufferFlags NOTIFY changed)
    Q_PROPERTY(QByteArray name MEMBER name NOTIFY changed)

    QML_NAMED_ELEMENT(Buffer)

public:
    QQuick3DShaderUtilsBuffer() = default;
    ~QQuick3DShaderUtilsBuffer() override = default;

    enum class TextureFilterOperation // must match QSSGRenderTextureFilterOp
    {
        Unknown = 0,
        Nearest,
        Linear
    };
    Q_ENUM(TextureFilterOperation)

    enum class TextureCoordOperation // must match QSSGRenderTextureCoordOp
    {
        Unknown = 0,
        ClampToEdge,
        MirroredRepeat,
        Repeat
    };
    Q_ENUM(TextureCoordOperation)

    enum class AllocateBufferFlagValues
    {
        None = 0,
        SceneLifetime = 1
    };
    Q_ENUM(AllocateBufferFlagValues)

    enum class TextureFormat {
        Unknown = 0,
        RGBA8,
        RGBA16F,
        RGBA32F,
        R8,
        R16,
        R16F,
        R32F
    };
    Q_ENUM(TextureFormat)

    QSSGAllocateBuffer command {};
    TextureFilterOperation textureFilterOperation() const { return TextureFilterOperation(command.m_filterOp); }
    void setTextureFilterOperation(TextureFilterOperation op);

    TextureCoordOperation textureCoordOperation() const { return TextureCoordOperation(command.m_texCoordOp); }
    void setTextureCoordOperation(TextureCoordOperation texCoordOp);
    float &sizeMultiplier = command.m_sizeMultiplier;
    QSSGCommand *cloneCommand() { return new QSSGAllocateBuffer(command); }

    TextureFormat format() const;
    void setFormat(TextureFormat format);

    AllocateBufferFlagValues bufferFlags() const { return AllocateBufferFlagValues(int(command.m_bufferFlags)); }
    void setBufferFlags(AllocateBufferFlagValues flag);

    QByteArray &name = command.m_name;

    static QSSGRenderTextureFormat::Format mapTextureFormat(QQuick3DShaderUtilsBuffer::TextureFormat fmt);
    static QQuick3DShaderUtilsBuffer::TextureFormat mapRenderTextureFormat(QSSGRenderTextureFormat::Format fmt);

Q_SIGNALS:
    void changed();
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderCommand : public QQuick3DObject
{
    Q_OBJECT

    QML_NAMED_ELEMENT(Command)

public:
    QQuick3DShaderUtilsRenderCommand() = default;
    ~QQuick3DShaderUtilsRenderCommand() override = default;
    virtual QSSGCommand *cloneCommand() { Q_ASSERT(0); return nullptr; }
    virtual int bufferCount() const { return 0; }
    virtual QQuick3DShaderUtilsBuffer *bufferAt(int idx) const { Q_UNUSED(idx); return nullptr; }
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsBufferInput : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DShaderUtilsBuffer *buffer READ buffer WRITE setBuffer)
    Q_PROPERTY(QByteArray sampler MEMBER sampler)

    QML_NAMED_ELEMENT(BufferInput)

public:
    QQuick3DShaderUtilsBufferInput() = default;
    ~QQuick3DShaderUtilsBufferInput() override = default;
    QSSGApplyBufferValue command { QByteArray(), QByteArray() };
    QByteArray &sampler = command.m_samplerName;
    QSSGCommand *cloneCommand() override { return new QSSGApplyBufferValue(command); }

    int bufferCount() const override { return (m_buffer != nullptr) ? 1 : 0; }
    QQuick3DShaderUtilsBuffer *bufferAt(int idx) const override
    {
        Q_ASSERT(idx < 1 && idx >= 0);
        return (m_buffer && idx == 0) ? m_buffer : nullptr;
    }

    QQuick3DShaderUtilsBuffer *buffer() const { return m_buffer; }
    void setBuffer(QQuick3DShaderUtilsBuffer *buffer) {
        if (m_buffer == buffer)
            return;

        if (buffer) {
            Q_ASSERT(!buffer->name.isEmpty());
            command.m_bufferName = buffer->name;
        }
        m_buffer = buffer;
    }

    QQuick3DShaderUtilsBuffer *m_buffer = nullptr;

};

class Q_QUICK3D_EXPORT QQuick3DRenderPassTargetBlend
{
    Q_GADGET
    Q_PROPERTY(bool enable MEMBER enable)
    Q_PROPERTY(ColorMask colorWrite MEMBER colorWrite)
    Q_PROPERTY(BlendFactor srcColor MEMBER srcColor)
    Q_PROPERTY(BlendFactor dstColor MEMBER dstColor)
    Q_PROPERTY(BlendOperation opColor MEMBER opColor)
    Q_PROPERTY(BlendFactor srcAlpha MEMBER srcAlpha)
    Q_PROPERTY(BlendFactor dstAlpha MEMBER dstAlpha)
    Q_PROPERTY(BlendOperation opAlpha MEMBER opAlpha)
    QML_VALUE_TYPE(renderTargetBlend)
    QML_ADDED_IN_VERSION(6, 11)

public:
    enum ColorMaskComponent : quint32 {
        R = 1 << 0,
        G = 1 << 1,
        B = 1 << 2,
        A = 1 << 3,
    };
    Q_DECLARE_FLAGS(ColorMask, ColorMaskComponent)

    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSaturate,
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha
    };
    Q_ENUM(BlendFactor)

    enum class BlendOperation {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };
    Q_ENUM(BlendOperation)

    ColorMask colorWrite = ColorMask(0xF); // R | G | B | A
    bool enable = false;
    BlendFactor srcColor = BlendFactor::One;
    BlendFactor dstColor = BlendFactor::OneMinusSrcAlpha;
    BlendOperation opColor = BlendOperation::Add;
    BlendFactor srcAlpha = BlendFactor::One;
    BlendFactor dstAlpha = BlendFactor::OneMinusSrcAlpha;
    BlendOperation opAlpha = BlendOperation::Add;

    bool operator==(const QQuick3DRenderPassTargetBlend &other) const
    {
        return colorWrite == other.colorWrite
            && enable == other.enable
            && srcColor == other.srcColor
            && dstColor == other.dstColor
            && opColor == other.opColor
            && srcAlpha == other.srcAlpha
            && dstAlpha == other.dstAlpha
            && opAlpha == other.opAlpha;
    }

    QQuick3DRenderPassTargetBlend() = default;
    QQuick3DRenderPassTargetBlend(QRhiGraphicsPipeline::TargetBlend targetBlend)
        : colorWrite(ColorMask(targetBlend.colorWrite.toInt()))
        , enable(targetBlend.enable)
        , srcColor(BlendFactor(targetBlend.srcColor))
        , dstColor(BlendFactor(targetBlend.dstColor))
        , opColor(BlendOperation(targetBlend.opColor))
        , srcAlpha(BlendFactor(targetBlend.srcAlpha))
        , dstAlpha(BlendFactor(targetBlend.dstAlpha))
        , opAlpha(BlendOperation(targetBlend.opAlpha))
    {
    }

    QRhiGraphicsPipeline::TargetBlend toRhiTargetBlend() const
    {
        QRhiGraphicsPipeline::TargetBlend tb;
        tb.colorWrite = QRhiGraphicsPipeline::ColorMask(colorWrite.toInt());
        tb.enable = enable;
        tb.srcColor = QRhiGraphicsPipeline::BlendFactor(int(srcColor));
        tb.dstColor = QRhiGraphicsPipeline::BlendFactor(int(dstColor));
        tb.opColor = QRhiGraphicsPipeline::BlendOp(int(opColor));
        tb.srcAlpha = QRhiGraphicsPipeline::BlendFactor(int(srcAlpha));
        tb.dstAlpha = QRhiGraphicsPipeline::BlendFactor(int(dstAlpha));
        tb.opAlpha = QRhiGraphicsPipeline::BlendOp(int(opAlpha));
        return tb;
    }
};

namespace QQuick3DRenderTargetBlendForeign
{
Q_NAMESPACE_EXPORT(Q_QUICK3D_EXPORT)
QML_FOREIGN_NAMESPACE(QQuick3DRenderPassTargetBlend)
QML_NAMED_ELEMENT(RenderTargetBlend)
QML_ADDED_IN_VERSION(6, 11)
}

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsPipelineStateOverride : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(bool depthTestEnabled READ depthTestEnabled WRITE setDepthTestEnabled RESET resetDepthTestEnabled NOTIFY depthTestEnabledChanged)
    Q_PROPERTY(bool depthWriteEnabled READ depthWriteEnabled WRITE setDepthWriteEnabled RESET resetDepthWriteEnabled NOTIFY depthWriteEnabledChanged)
    Q_PROPERTY(bool blendEnabled READ blendEnabled WRITE setBlendEnabled RESET resetBlendEnabled NOTIFY blendEnabledChanged FINAL)
    Q_PROPERTY(bool usesStencilReference READ usesStencilReference WRITE setUsesStencilReference RESET resetUsesStencilReference NOTIFY usesStencilReferenceChanged)
    Q_PROPERTY(bool usesScissor READ usesScissor WRITE setUsesScissor RESET resetUsesScissor NOTIFY usesScissorChanged)
    Q_PROPERTY(CompareOperation depthFunction READ depthFunction WRITE setDepthFunction RESET resetDepthFunction NOTIFY depthFunctionChanged)
    Q_PROPERTY(CullMode cullMode READ cullMode WRITE setCullMode RESET resetCullMode NOTIFY cullModeChanged)
    Q_PROPERTY(PolygonMode polygonMode READ polygonMode WRITE setPolygonMode RESET resetPolygonMode NOTIFY polygonModeChanged)
    Q_PROPERTY(quint32 stencilWriteMask READ stencilWriteMask WRITE setStencilWriteMask RESET resetStencilWriteMask NOTIFY stencilWriteMaskChanged)
    Q_PROPERTY(quint32 stencilReference READ stencilReference WRITE setStencilReference RESET resetStencilReference NOTIFY stencilReferenceChanged)
    Q_PROPERTY(QRectF viewport READ viewport WRITE setViewport RESET resetViewport NOTIFY viewportChanged)
    Q_PROPERTY(QRect scissor READ scissor WRITE setScissor RESET resetScissor NOTIFY scissorChanged)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend0 READ targetBlend0 WRITE setTargetBlend0 RESET resetTargetBlend0 NOTIFY targetBlend0Changed)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend1 READ targetBlend1 WRITE setTargetBlend1 RESET resetTargetBlend1 NOTIFY targetBlend1Changed)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend2 READ targetBlend2 WRITE setTargetBlend2 RESET resetTargetBlend2 NOTIFY targetBlend2Changed)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend3 READ targetBlend3 WRITE setTargetBlend3 RESET resetTargetBlend3 NOTIFY targetBlend3Changed)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend4 READ targetBlend4 WRITE setTargetBlend4 RESET resetTargetBlend4 NOTIFY targetBlend4Changed)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend5 READ targetBlend5 WRITE setTargetBlend5 RESET resetTargetBlend5 NOTIFY targetBlend5Changed)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend6 READ targetBlend6 WRITE setTargetBlend6 RESET resetTargetBlend6 NOTIFY targetBlend6Changed)
    Q_PROPERTY(QQuick3DRenderPassTargetBlend targetBlend7 READ targetBlend7 WRITE setTargetBlend7 RESET resetTargetBlend7 NOTIFY targetBlend7Changed)
    QML_NAMED_ELEMENT(PipelineStateOverride)
    QML_ADDED_IN_VERSION(6, 11)

public:
    enum class CompareOperation {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };
    Q_ENUM(CompareOperation)

    enum class CullMode {
        None,
        Front,
        Back
    };
    Q_ENUM(CullMode)

    enum class PolygonMode {
        Fill,
        Line
    };
    Q_ENUM(PolygonMode)

    QQuick3DShaderUtilsPipelineStateOverride() = default;
    ~QQuick3DShaderUtilsPipelineStateOverride() override;

    bool depthTestEnabled() const;
    void setDepthTestEnabled(bool newDepthTestEnabled);
    void resetDepthTestEnabled();
    bool depthWriteEnabled() const;
    void setDepthWriteEnabled(bool newDepthWriteEnabled);
    void resetDepthWriteEnabled();

    bool blendEnabled() const;
    void setBlendEnabled(bool newBlendEnabled);
    void resetBlendEnabled();

    bool usesStencilReference() const;
    void setUsesStencilReference(bool newUsesStencilReference);
    void resetUsesStencilReference();

    bool usesScissor() const;
    void setUsesScissor(bool newUsesScissor);
    void resetUsesScissor();

    CompareOperation depthFunction() const;
    void setDepthFunction(CompareOperation newDepthFunction);
    void resetDepthFunction();

    CullMode cullMode() const;
    void setCullMode(CullMode newCullMode);
    void resetCullMode();

    PolygonMode polygonMode() const;
    void setPolygonMode(PolygonMode newPolygonMode);
    void resetPolygonMode();

    quint32 stencilWriteMask() const;
    void setStencilWriteMask(quint32 newStencilWriteMask);
    void resetStencilWriteMask();

    quint32 stencilReference() const;
    void setStencilReference(quint32 newStencilReference);
    void resetStencilReference();

    QRectF viewport() const;
    void setViewport(const QRectF &newViewport);
    void resetViewport();

    QRect scissor() const;
    void setScissor(const QRect &newScissor);
    void resetScissor();

    QQuick3DRenderPassTargetBlend targetBlend0() const;
    void setTargetBlend0(const QQuick3DRenderPassTargetBlend &newTargetBlend0);
    void resetTargetBlend0();

    QQuick3DRenderPassTargetBlend targetBlend1() const;
    void setTargetBlend1(const QQuick3DRenderPassTargetBlend &newTargetBlend1);
    void resetTargetBlend1();

    QQuick3DRenderPassTargetBlend targetBlend2() const;
    void setTargetBlend2(const QQuick3DRenderPassTargetBlend &newTargetBlend2);
    void resetTargetBlend2();

    QQuick3DRenderPassTargetBlend targetBlend3() const;
    void setTargetBlend3(const QQuick3DRenderPassTargetBlend &newTargetBlend3);
    void resetTargetBlend3();

    QQuick3DRenderPassTargetBlend targetBlend4() const;
    void setTargetBlend4(const QQuick3DRenderPassTargetBlend &newTargetBlend4);
    void resetTargetBlend4();

    QQuick3DRenderPassTargetBlend targetBlend5() const;
    void setTargetBlend5(const QQuick3DRenderPassTargetBlend &newTargetBlend5);
    void resetTargetBlend5();

    QQuick3DRenderPassTargetBlend targetBlend6() const;
    void setTargetBlend6(const QQuick3DRenderPassTargetBlend &newTargetBlend6);
    void resetTargetBlend6();

    QQuick3DRenderPassTargetBlend targetBlend7() const;
    void setTargetBlend7(const QQuick3DRenderPassTargetBlend &newTargetBlend7);
    void resetTargetBlend7();

Q_SIGNALS:
    void depthTestEnabledChanged();
    void depthWriteEnabledChanged();
    void blendEnabledChanged();
    void usesStencilReferenceChanged();
    void usesScissorChanged();
    void depthFunctionChanged();
    void cullModeChanged();
    void polygonModeChanged();
    void stencilWriteMaskChanged();
    void stencilReferenceChanged();
    void viewportChanged();
    void scissorChanged();
    void targetBlend0Changed();
    void targetBlend1Changed();
    void targetBlend2Changed();
    void targetBlend3Changed();
    void targetBlend4Changed();
    void targetBlend5Changed();
    void targetBlend6Changed();
    void targetBlend7Changed();

private:
    QSSGCommand *cloneCommand() override { return new QSSGPipelineStateOverrideCommand(command); }
    QSSGPipelineStateOverrideCommand command { };
};


class Q_QUICK3D_EXPORT QQuick3DShaderUtilsApplyValue : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QByteArray target MEMBER target)
    Q_PROPERTY(QVariant value MEMBER value)

    QML_NAMED_ELEMENT(SetUniformValue)

public:
    QQuick3DShaderUtilsApplyValue() = default;
    ~QQuick3DShaderUtilsApplyValue() override = default;
    QSSGCommand *cloneCommand() override { return new QSSGApplyValue(command); }
    QSSGApplyValue command { };
    QVariant &value = command.m_value;
    QByteArray &target = command.m_propertyName;
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderablesFilter : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT

    Q_PROPERTY(quint32 layerMask MEMBER layerMask)
    Q_PROPERTY(RenderableTypes renderableTypes READ renderableTypes WRITE setRenderableTypes)
    QML_NAMED_ELEMENT(RenderablesFilter)
    QML_ADDED_IN_VERSION(6, 11)

public:
    enum class RenderableType : quint32 {
        None = 0x0,
        Opaque = 0x1,
        Transparent = 0x2,
    };
    Q_DECLARE_FLAGS(RenderableTypes, RenderableType)
    Q_FLAG(RenderableTypes)

    QQuick3DShaderUtilsRenderablesFilter() = default;
    ~QQuick3DShaderUtilsRenderablesFilter() override;

    RenderableTypes renderableTypes() const;
    void setRenderableTypes(RenderableTypes types);

    QSSGCommand *cloneCommand() override { return new QSSGRenderablesFilterCommand(command); }
    QSSGRenderablesFilterCommand command { };

    quint32 &layerMask = command.layerMask;
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderPassTexture : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(TextureFormat format READ format WRITE setFormat FINAL)
    QML_NAMED_ELEMENT(RenderPassTexture)
    QML_ADDED_IN_VERSION(6, 11)

public:
    enum class TextureFormat {
        Unknown = 0,
        RGBA8,
        RGBA16F,
        RGBA32F,
        R8,
        R16,
        R16F,
        R32F,
        Depth16,
        Depth24,
        Depth32,
        Depth24Stencil8,
    };
    Q_ENUM(TextureFormat)

    QQuick3DShaderUtilsRenderPassTexture() = default;
    ~QQuick3DShaderUtilsRenderPassTexture() override;

    TextureFormat format() const;
    void setFormat(TextureFormat newFormat);

    std::shared_ptr<QSSGAllocateTexture> command;

private:
    friend class QQuick3DShaderUtilsRenderPassColorAttachment;
    friend class QQuick3DShaderUtilsRenderPassDepthTextureAttachment;


private:
    static QSSGRenderTextureFormat asRenderTextureFormat(QQuick3DShaderUtilsRenderPassTexture::TextureFormat fmt);
    static QQuick3DShaderUtilsRenderPassTexture::TextureFormat fromRenderTextureFormat(QSSGRenderTextureFormat fmt);
};

// Class for user defined color attachments in render passes
class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderPassColorAttachment : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DShaderUtilsRenderPassTexture *target MEMBER target)
    Q_PROPERTY(QByteArray name READ name WRITE setName FINAL)
    QML_NAMED_ELEMENT(ColorAttachment)
    QML_ADDED_IN_VERSION(6, 11)

public:
    QQuick3DShaderUtilsRenderPassColorAttachment() = default;
    ~QQuick3DShaderUtilsRenderPassColorAttachment() override;

    QByteArray name() const;
    void setName(const QByteArray &newName);

    QSSGCommand *cloneCommand() override;

private:
    QPointer<QQuick3DShaderUtilsRenderPassTexture> target;
    QByteArray m_name;
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderPassDepthTextureAttachment : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DShaderUtilsRenderPassTexture *target MEMBER target)
    QML_NAMED_ELEMENT(DepthTextureAttachment)
    QML_ADDED_IN_VERSION(6, 11)

public:
    QQuick3DShaderUtilsRenderPassDepthTextureAttachment() = default;
    ~QQuick3DShaderUtilsRenderPassDepthTextureAttachment() override;

    QSSGCommand *cloneCommand() override;

    QPointer<QQuick3DShaderUtilsRenderPassTexture> target;
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderPassDepthStencilAttachment : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    QML_NAMED_ELEMENT(DepthStencilAttachment)
    QML_ADDED_IN_VERSION(6, 11)

public:
    QQuick3DShaderUtilsRenderPassDepthStencilAttachment() = default;
    ~QQuick3DShaderUtilsRenderPassDepthStencilAttachment() override;

    QSSGCommand *cloneCommand() override;

};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderPassAddDefine : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QByteArray name MEMBER name)
    Q_PROPERTY(int value MEMBER value)
    QML_NAMED_ELEMENT(AddDefine)
    QML_ADDED_IN_VERSION(6, 11)

public:
    QQuick3DShaderUtilsRenderPassAddDefine();
    ~QQuick3DShaderUtilsRenderPassAddDefine() override;

    QSSGCommand *cloneCommand() override;

    QSSGAddShaderDefine command;
    QByteArray &name = command.m_name;
    int &value = command.m_value;
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsSubRenderPass : public QQuick3DShaderUtilsRenderCommand
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DRenderPass *renderPass READ renderPass WRITE setRenderPass NOTIFY renderPassChanged FINAL)
    QML_NAMED_ELEMENT(SubRenderPass)
    QML_ADDED_IN_VERSION(6, 11)

public:
    QQuick3DShaderUtilsSubRenderPass() = default;
    ~QQuick3DShaderUtilsSubRenderPass() override;

    QQuick3DRenderPass *renderPass() const;
    void setRenderPass(QQuick3DRenderPass *newRenderPass);

    QSSGCommand *cloneCommand() override;

signals:
    void renderPassChanged();

protected:
    virtual QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) final;
    virtual void itemChange(ItemChange change, const ItemChangeData &value) final;

private:
    void updateSceneManager(QQuick3DSceneManager *sceneManager);

    QQuick3DRenderPass *m_renderPass = nullptr;
    mutable bool m_hasWarnedAboutInvalidId = false;
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsRenderPass : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> commands READ commands)
    Q_PROPERTY(QQuick3DShaderUtilsBuffer *output MEMBER outputBuffer)
    Q_PROPERTY(QQmlListProperty<QQuick3DShaderUtilsShader> shaders READ shaders)
    QML_NAMED_ELEMENT(Pass)
    QML_ADDED_IN_VERSION(6, 11)

public:
    QQuick3DShaderUtilsRenderPass() = default;
    ~QQuick3DShaderUtilsRenderPass() override;

    static void qmlAppendCommand(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list, QQuick3DShaderUtilsRenderCommand *command);
    static QQuick3DShaderUtilsRenderCommand *qmlCommandAt(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list, qsizetype index);
    static qsizetype qmlCommandCount(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list);
    static void qmlCommandClear(QQmlListProperty<QQuick3DShaderUtilsRenderCommand> *list);

    static void qmlAppendShader(QQmlListProperty<QQuick3DShaderUtilsShader> *list, QQuick3DShaderUtilsShader *shader);
    static QQuick3DShaderUtilsShader *qmlShaderAt(QQmlListProperty<QQuick3DShaderUtilsShader> *list, qsizetype index);
    static qsizetype qmlShaderCount(QQmlListProperty<QQuick3DShaderUtilsShader> *list);
    static void qmlShaderClear(QQmlListProperty<QQuick3DShaderUtilsShader> *list);

    QQmlListProperty<QQuick3DShaderUtilsRenderCommand> commands();
    QVector<QQuick3DShaderUtilsRenderCommand *> m_commands;
    QQuick3DShaderUtilsBuffer *outputBuffer = nullptr;
    QQmlListProperty<QQuick3DShaderUtilsShader> shaders();
    QVarLengthArray<QQuick3DShaderUtilsShader *, 2> m_shaders;

Q_SIGNALS:
    void changed();
};

class Q_QUICK3D_EXPORT QQuick3DShaderUtilsShader : public QQuick3DObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl shader MEMBER shader NOTIFY shaderChanged)
    Q_PROPERTY(Stage stage MEMBER stage NOTIFY stageChanged)

    QML_NAMED_ELEMENT(Shader)

public:
    QQuick3DShaderUtilsShader() = default;
    virtual ~QQuick3DShaderUtilsShader() = default;
    enum class Stage : quint8
    {
        Vertex = 0,
        Fragment = 1
    };
    Q_ENUM(Stage)

    QUrl shader;
    Stage stage = Stage::Fragment;

Q_SIGNALS:
    void shaderChanged();
    void stageChanged();
};

template<typename T, typename std::enable_if_t<std::is_base_of_v<QQuick3DObject, T>, bool> = true>
class QQuick3DSuperClassInfo
{
public:
    QQuick3DSuperClassInfo() = default;

    static const char *superClassName()
    {
        return T::staticMetaObject.className();
    }
};

class QQuick3DPropertyChangedTracker;

class QQuick3DPropertyWatcher : public QObject
{
    Q_OBJECT
public:
    QQuick3DPropertyWatcher(QQuick3DPropertyChangedTracker *tracker, QMetaProperty property);

public Q_SLOTS:
    void onValuePropertyChanged();
    void onPointerPropertyChanged();

private:
    QQuick3DPropertyChangedTracker *m_tracker = nullptr;
    QMetaProperty m_property;
};

class QQuick3DPropertyChangedTracker
{
public:
    enum class DirtyPropertyHint
    {
        Value,
        Reference,
    };

    template<typename T>
    explicit QQuick3DPropertyChangedTracker(QQuick3DObject *owner, QQuick3DSuperClassInfo<T> info)
        : m_owner(owner)
        , m_superClassName(info.superClassName())
    {
        Q_ASSERT(m_owner != nullptr);
    }
    virtual ~QQuick3DPropertyChangedTracker();

    using UniformProperty = QSSGUserShaderAugmentation::Property;
    using UniformPropertyList = QVector<UniformProperty>;
protected:
    friend class QQuick3DPropertyWatcher;

    void extractProperties(UniformPropertyList &outUniforms);

    void addPropertyWatcher(QMetaProperty property, DirtyPropertyHint hint, QQuick3DObject *object = nullptr);

    virtual void markTrackedPropertyDirty(QMetaProperty property, DirtyPropertyHint hint) = 0;

    struct Tracked
    {
        QQuick3DPropertyWatcher *watcher = nullptr;
        QPointer<QQuick3DObject> object;
        int pid = -1;
    };

    using TrackedProperties = std::vector<Tracked>;
    TrackedProperties m_trackedProperties;
    QQuick3DObject *m_owner = nullptr;
    const char *m_superClassName = nullptr;
};

QT_END_NAMESPACE

Q_DECLARE_OPAQUE_POINTER(QQuick3DShaderUtilsTextureInput)

#endif // QQUICK3DSHADERUTILS_H

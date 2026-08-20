// Copyright (C) 2008-2012 NVIDIA Corporation.
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_COMMANDS_H
#define QSSG_RENDER_COMMANDS_H

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

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderuserpass_p.h>

#include <QDebug>
#include <QVariant>

QT_BEGIN_NAMESPACE

struct QSSGRenderEffect;

enum class CommandType
{
    Unknown = 0,
    AllocateBuffer,
    AllocateTexture,
    BindTarget,
    BindBuffer,
    BindShader,
    ApplyInstanceValue,
    ApplyBufferValue,
    Render,
    ApplyValue,
    RenderablesFilter,
    PipelineStateOverride,
    ColorAttachment,
    DepthStencilAttachment,
    DepthTextureAttachment,
    AddShaderDefine,
    SubRenderPass
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGCommand
{
public:
    CommandType m_type;
    QSSGCommand(CommandType inType) : m_type(inType) {}
    virtual ~QSSGCommand();
    const char *typeAsString() const;
    QString debugString() const;
    void addDebug(QDebug &stream) const;
};

enum class AllocateBufferFlagValues
{
    None = 0,
    SceneLifetime = 1,
};

struct QSSGAllocateBufferFlags : public QFlags<AllocateBufferFlagValues>
{
    QSSGAllocateBufferFlags(quint32 inValues) : QFlags(inValues) {}
    QSSGAllocateBufferFlags() {}
    void setSceneLifetime(bool inValue) { setFlag(AllocateBufferFlagValues::SceneLifetime, inValue); }
    // If isSceneLifetime is unset the buffer is assumed to be frame lifetime and will be
    // released after this render operation.
    bool isSceneLifetime() const { return this->operator&(AllocateBufferFlagValues::SceneLifetime); }
};

struct QSSGAllocateBuffer : public QSSGCommand
{
    QByteArray m_name;
    QSSGRenderTextureFormat m_format = QSSGRenderTextureFormat::RGBA8;
    QSSGRenderTextureFilterOp m_filterOp = QSSGRenderTextureFilterOp::Linear;
    QSSGRenderTextureCoordOp m_texCoordOp = QSSGRenderTextureCoordOp::ClampToEdge;
    float m_sizeMultiplier = 1.0f;
    QSSGAllocateBufferFlags m_bufferFlags;
    QSSGAllocateBuffer() : QSSGCommand(CommandType::AllocateBuffer) {}
    QSSGAllocateBuffer(const QByteArray &inName,
                         QSSGRenderTextureFormat inFormat,
                         QSSGRenderTextureFilterOp inFilterOp,
                         QSSGRenderTextureCoordOp inCoordOp,
                         float inMultiplier,
                         QSSGAllocateBufferFlags inFlags)
        : QSSGCommand(CommandType::AllocateBuffer)
        , m_name(inName)
        , m_format(inFormat)
        , m_filterOp(inFilterOp)
        , m_texCoordOp(inCoordOp)
        , m_sizeMultiplier(inMultiplier)
        , m_bufferFlags(inFlags)
    {
    }
    QSSGAllocateBuffer(const QSSGAllocateBuffer &command)
        : QSSGCommand(CommandType::AllocateBuffer),
          m_name(command.m_name),
          m_format(command.m_format),
          m_filterOp(command.m_filterOp),
          m_texCoordOp(command.m_texCoordOp),
          m_sizeMultiplier(command.m_sizeMultiplier),
          m_bufferFlags(command.m_bufferFlags)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "name:" <<  m_name << "format:" << m_format.toString() << "size multiplier:" << m_sizeMultiplier << "filter:" << QSSGBaseTypeHelpers::toString(m_filterOp) << "tiling:" << QSSGBaseTypeHelpers::toString(m_texCoordOp) << "sceneLifetime:" << m_bufferFlags.isSceneLifetime();
    }
};

struct QSSGBindTarget : public QSSGCommand
{
    QSSGRenderTextureFormat m_outputFormat;

    explicit QSSGBindTarget(QSSGRenderTextureFormat inFormat = QSSGRenderTextureFormat::RGBA8)
        : QSSGCommand(CommandType::BindTarget), m_outputFormat(inFormat)
    {
    }
    QSSGBindTarget(const QSSGBindTarget &command)
        : QSSGCommand(CommandType::BindTarget),
          m_outputFormat(command.m_outputFormat)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "format" <<  m_outputFormat.toString();
    }
};

struct QSSGBindBuffer : public QSSGCommand
{
    QByteArray m_bufferName;
    QSSGBindBuffer(const QByteArray &inBufName)
        : QSSGCommand(CommandType::BindBuffer), m_bufferName(inBufName)
    {
    }
    QSSGBindBuffer(const QSSGBindBuffer &command)
        : QSSGCommand(CommandType::BindBuffer),
          m_bufferName(command.m_bufferName)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "name:" <<  m_bufferName;
    }
};

struct QSSGBindShader : public QSSGCommand
{
    QByteArray m_shaderPathKey; // something like "prefix>vertex_filename>fragment_filename:source_sha:y_up_in_fbo[:tonemapping]"
    QSSGBindShader(const QByteArray &inShaderPathKey)
        : QSSGCommand(CommandType::BindShader),
          m_shaderPathKey(inShaderPathKey)
    {
    }
    QSSGBindShader() : QSSGCommand(CommandType::BindShader) {}
    QSSGBindShader(const QSSGBindShader &command)
        : QSSGCommand(CommandType::BindShader),
          m_shaderPathKey(command.m_shaderPathKey)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "key:" <<  m_shaderPathKey << "effect:";
    }
};

// The value sits immediately after the 'this' object
// in memory.
// If propertyName is not valid then we attempt to apply all of the effect property values
// to the shader, ignoring ones that don't match up.
struct QSSGApplyInstanceValue : public QSSGCommand
{
    // Name of value to apply in shader
    QByteArray m_propertyName;
    // type of value
    QSSGRenderShaderValue::Type m_valueType;
    // offset in the effect data section of value.
    quint32 m_valueOffset;
    QSSGApplyInstanceValue(const QByteArray &inName, QSSGRenderShaderValue::Type inValueType, quint32 inValueOffset)
        : QSSGCommand(CommandType::ApplyInstanceValue), m_propertyName(inName), m_valueType(inValueType), m_valueOffset(inValueOffset)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    QSSGApplyInstanceValue()
        : QSSGCommand(CommandType::ApplyInstanceValue), m_valueType(QSSGRenderShaderValue::Unknown), m_valueOffset(0)
    {
    }
    QSSGApplyInstanceValue(const QSSGApplyInstanceValue &command)
        : QSSGCommand(CommandType::ApplyInstanceValue),
          m_propertyName(command.m_propertyName),
          m_valueType(command.m_valueType),
          m_valueOffset(command.m_valueOffset)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "name:" <<  m_propertyName << "type:" << int(m_valueType) << "offset:" << m_valueOffset ;
    }
};

struct QSSGApplyValue : public QSSGCommand
{
    QByteArray m_propertyName;
    QVariant m_value;
    explicit QSSGApplyValue(const QByteArray &inName)
        : QSSGCommand(CommandType::ApplyValue), m_propertyName(inName)
    {
    }
    QSSGApplyValue(const QSSGApplyValue &command)
        : QSSGCommand(CommandType::ApplyValue),
          m_propertyName(command.m_propertyName),
          m_value(command.m_value)
    {
    }
    // Default will attempt to apply all effect values to the currently bound shader
    QSSGApplyValue() : QSSGCommand(CommandType::ApplyValue) {}
    void addDebug(QDebug &stream) const {
        stream << "name:" <<  m_propertyName << "value:" << m_value;
    }
};

struct QSSGApplyBufferValue : public QSSGCommand
{
    QByteArray m_bufferName;
    QByteArray m_samplerName;

    QSSGApplyBufferValue(const QByteArray &bufferName, const QByteArray &shaderSampler)
        : QSSGCommand(CommandType::ApplyBufferValue), m_bufferName(bufferName), m_samplerName(shaderSampler)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "name:" <<  m_bufferName << "sampler:" << m_samplerName;
    }
    QSSGApplyBufferValue(const QSSGApplyBufferValue &command)
        : QSSGCommand(CommandType::ApplyBufferValue),
          m_bufferName(command.m_bufferName),
          m_samplerName(command.m_samplerName)
    {
    }
};

struct QSSGRender : public QSSGCommand
{
    explicit QSSGRender() : QSSGCommand(CommandType::Render) { }

    QSSGRender(const QSSGRender &)
        : QSSGCommand(CommandType::Render)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "(no parameters)";
    }
};

class QSSGRenderablesFilterCommand : public QSSGCommand
{
public:
    enum class RenderableType : quint32 {
        None = 0x0,
        Opaque = 0x1,
        Transparent = 0x2,
    };

    using RenderableTypeT = std::underlying_type<RenderableType>::type;
    static constexpr RenderableTypeT AllRenderableTypes = std::numeric_limits<RenderableTypeT>::max();

    quint32 layerMask = 0xffffffff;
    RenderableTypeT renderableTypes = AllRenderableTypes;

    QSSGRenderablesFilterCommand()
        : QSSGCommand(CommandType::RenderablesFilter)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "layerMask:" <<  layerMask << "renderableTypes:" << renderableTypes;
    }
};

// This is pretty much the same as QSSGAllocateBuffer but
// we'll keep them separate as we might want to diverge further later.
class QSSGAllocateTexture : public QSSGCommand
{
public:
    QSSGAllocateTexture()
        : QSSGCommand(CommandType::AllocateTexture)
    {
    }
    QSSGAllocateTexture(QSSGRenderTextureFormat inFormat)
        : QSSGCommand(CommandType::AllocateTexture),
          m_format(inFormat)
    {
    }

    const QSSGManagedRhiTexturePtr &texture() const { return m_tex; }
    void setTexture(const QSSGManagedRhiTexturePtr &tex) { m_tex = QSSGManagedRhiTexture::make_copy(tex); }

    QSSGRenderTextureFormat format() const
    {
        return m_format;
    }

    void setFormat(QSSGRenderTextureFormat format)
    {
        m_format = format;
    }

    void addDebug(QDebug &stream) const
    {
        stream << "format:" << m_format.toString();
    }

private:
    Q_DISABLE_COPY_MOVE(QSSGAllocateTexture)

    QSSGManagedRhiTexturePtr m_tex;
    QSSGRenderTextureFormat m_format = QSSGRenderTextureFormat::RGBA8;
};

using QSSGAllocateTexturePtr = std::shared_ptr<QSSGAllocateTexture>;

class QSSGColorAttachment : public QSSGCommand
{
public:
    QSSGColorAttachment(const QByteArray &name)
        : QSSGCommand(CommandType::ColorAttachment)
        , m_name(name)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "name:" <<  m_name;
    }

    QSSGRenderTextureFormat format() const
    {
        if (m_textureCmd)
            return m_textureCmd->format();

        return QSSGRenderTextureFormat::Unknown;
    }

    QByteArray m_name;
    QSSGAllocateTexturePtr m_textureCmd;

protected:
    QSSGColorAttachment(const QByteArray &name, CommandType type)
        : QSSGCommand(type)
        , m_name(name)
    {
    }
};

class QSSGDepthTextureAttachment : public QSSGColorAttachment
{
public:
    QSSGDepthTextureAttachment(const QByteArray &name)
        : QSSGColorAttachment(name, CommandType::DepthTextureAttachment)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "name:" <<  m_name;
    }
};

class QSSGDepthStencilAttachment : public QSSGCommand
{
public:
    QSSGDepthStencilAttachment()
        : QSSGCommand(CommandType::DepthStencilAttachment)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "(no parameters)";
    }

    QRhiTexture::Format m_format = QRhiTexture::D24S8;
};

class QSSGAddShaderDefine : public QSSGCommand
{
public:
    QSSGAddShaderDefine()
        : QSSGCommand(CommandType::AddShaderDefine)
        , m_value(0)
    {
    }
    QSSGAddShaderDefine(const QByteArray &name, int value = 0)
        : QSSGCommand(CommandType::AddShaderDefine)
        , m_name(name)
        , m_value(value)
    {
    }
    void addDebug(QDebug &stream) const {
        stream << "define name:" <<  m_name << "value:" << m_value;
    }

    QByteArray m_name;
    int m_value;
};

class QSSGSubRenderPass : public QSSGCommand
{
public:
    QSSGSubRenderPass()
        : QSSGCommand(CommandType::SubRenderPass)
    {
    }

    void setSubPass(QSSGResourceId userPassId) {
        m_userPassId = userPassId;
    }

    void addDebug(QDebug &stream) const {
        stream << "userPassId:" <<  static_cast<quint64>(m_userPassId);
    }

    QSSGResourceId m_userPassId = QSSGResourceId::Invalid;
};

class QSSGPipelineStateOverrideCommand : public QSSGCommand
{
public:
    std::optional<bool> m_depthTestEnabled;
    std::optional<bool> m_depthWriteEnabled;
    std::optional<bool> m_blendEnabled;
    std::optional<bool> m_usesStencilReference;
    std::optional<bool> m_usesScissor;
    std::optional<QRhiGraphicsPipeline::CompareOp> m_depthFunction;
    std::optional<QRhiGraphicsPipeline::CullMode> m_cullMode;
    std::optional<QRhiGraphicsPipeline::PolygonMode> m_polygonMode;
    std::optional<QRhiGraphicsPipeline::StencilOpState> m_stencilOpFrontState;
    std::optional<quint32> m_stencilWriteMask;
    std::optional<quint32> m_stencilReference;
    std::optional<QRhiViewport> m_viewport;
    std::optional<QRhiScissor> m_scissor;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend0;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend1;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend2;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend3;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend4;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend5;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend6;
    std::optional<QRhiGraphicsPipeline::TargetBlend> m_targetBlend7;
    QSSGPipelineStateOverrideCommand()
        : QSSGCommand(CommandType::PipelineStateOverride)
    {
    }
    void addDebug(QDebug &stream) const {
        // Only print out the ones that exist
        // Elipsis for complex types intentional
        stream << "pipelineState:" << "{";
        if (m_depthTestEnabled.has_value())
            stream << " depthTestEnabled:" << m_depthTestEnabled.value();
        if (m_depthWriteEnabled.has_value())
            stream << " depthWriteEnabled:" << m_depthWriteEnabled.value();
        if (m_blendEnabled.has_value())
            stream << " blendEnabled:" << m_blendEnabled.value();
        if (m_usesStencilReference.has_value())
            stream << " usesStencilReference:" << m_usesStencilReference.value();
        if (m_usesScissor.has_value())
            stream << " usesScissor:" << m_usesScissor.value();
        if (m_depthFunction.has_value())
            stream << " depthFunction:" << int(m_depthFunction.value());
        if (m_cullMode.has_value())
            stream << " cullMode:" << int(m_cullMode.value());
        if (m_polygonMode.has_value())
            stream << " polygonMode:" << int(m_polygonMode.value());
        if (m_stencilOpFrontState.has_value())
            stream << " stencilOpFrontState:" << "{...}";
        if (m_stencilWriteMask.has_value())
            stream << " stencilWriteMask:" << m_stencilWriteMask.value();
        if (m_stencilReference.has_value())
            stream << " stencilReference:" << m_stencilReference.value();
        if (m_viewport.has_value())
            stream << " viewport:" << "{...}";
        if (m_scissor.has_value())
            stream << " scissor:" << "{...}";
        if (m_targetBlend0.has_value())
            stream << " targetBlend0:" << "{...}";
        if (m_targetBlend1.has_value())
            stream << " targetBlend1:" << "{...}";
        if (m_targetBlend2.has_value())
            stream << " targetBlend2:" << "{...}";
        if (m_targetBlend3.has_value())
            stream << " targetBlend3:" << "{...}";
        if (m_targetBlend4.has_value())
            stream << " targetBlend4:" << "{...}";
        if (m_targetBlend5.has_value())
            stream << " targetBlend5:" << "{...}";
        if (m_targetBlend6.has_value())
            stream << " targetBlend6:" << "{...}";
        if (m_targetBlend7.has_value())
            stream << " targetBlend7:" << "{...}";
        stream << " }";
    }
};

QT_END_NAMESPACE

#endif

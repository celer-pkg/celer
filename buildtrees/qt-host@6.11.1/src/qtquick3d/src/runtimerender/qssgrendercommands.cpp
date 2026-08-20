// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrendercommands_p.h"

QSSGCommand::~QSSGCommand()
{

}

const char *QSSGCommand::typeAsString() const
{
    switch (m_type) {
    case CommandType::AddShaderDefine:
        return "AddShaderDefine";
    case CommandType::DepthTextureAttachment:
        return "DepthTextureAttachment";
    case CommandType::ColorAttachment:
        return "ColorAttachment";
    case CommandType::AllocateTexture:
        return "AllocateTexture";
    case CommandType::RenderablesFilter:
        return "RenderablesFilter";
    case CommandType::Unknown:
        return "Unknown";
    case CommandType::AllocateBuffer:
        return "AllocateBuffer";
    case CommandType::BindTarget:
        return "BindTarget";
    case CommandType::BindBuffer:
        return "BindBuffer";
    case CommandType::BindShader:
        return "BindShader";
    case CommandType::ApplyInstanceValue:
        return "ApplyInstanceValue";
    case CommandType::ApplyBufferValue:
        return "ApplyBufferValue";
    case CommandType::Render:
        return "Render";
    case CommandType::ApplyValue:
        return "ApplyValue";
    case CommandType::PipelineStateOverride:
        return "PipelineStateOverride";
    case CommandType::DepthStencilAttachment:
        return "DepthStencilAttachment";
    case CommandType::SubRenderPass:
        return "SubRenderPass";
    }

    Q_UNREACHABLE_RETURN("");
}

QString QSSGCommand::debugString() const
{
    QString result;
    QDebug stream(&result);

    switch (m_type) {
    case CommandType::AllocateTexture:
        static_cast<const QSSGAllocateTexture *>(this)->addDebug(stream);
        break;
    case CommandType::AllocateBuffer:
        static_cast<const QSSGAllocateBuffer*>(this)->addDebug(stream);
        break;
    case CommandType::BindTarget:
        static_cast<const QSSGBindTarget*>(this)->addDebug(stream);
        break;
    case CommandType::BindBuffer:
        static_cast<const QSSGBindBuffer*>(this)->addDebug(stream);
        break;
    case CommandType::BindShader:
        static_cast<const QSSGBindShader*>(this)->addDebug(stream);
        break;
    case CommandType::ApplyInstanceValue:
        static_cast<const QSSGApplyInstanceValue*>(this)->addDebug(stream);
        break;
    case CommandType::ApplyBufferValue:
        static_cast<const QSSGApplyBufferValue*>(this)->addDebug(stream);
        break;
    case CommandType::Render:
        static_cast<const QSSGRender*>(this)->addDebug(stream);
        break;
    case CommandType::ApplyValue:
        static_cast<const QSSGApplyValue*>(this)->addDebug(stream);
        break;
    case CommandType::Unknown:
    default:
        addDebug(stream);
        break;
    }

    return result;
}

void QSSGCommand::addDebug(QDebug &stream) const
{
    stream << "No debug info for " << typeAsString();
}

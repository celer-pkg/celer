// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrhicontext_p.h"

#include <QtCore/qvariant.h>
#include <QtGui/private/qrhi_p.h>

#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>
#include <QtQuick3DUtils/private/qssgmesh_p.h>
#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderableimage_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermesh_p.h>
#include <QtQuick3DRuntimeRender/private/qssguserrenderpassmanager_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3DUtils/private/qssgassert_p.h>
#include <qtquick3d_tracepoints_p.h>

#include <QtGui/qquaternion.h>

#include <memory>

QT_BEGIN_NAMESPACE

Q_TRACE_POINT(qtquick3d, QSSG_renderPass_entry, const QString &renderPass);
Q_TRACE_POINT(qtquick3d, QSSG_renderPass_exit);
Q_TRACE_POINT(qtquick3d, QSSG_drawIndexed, int indexCount, int instanceCount);
Q_TRACE_POINT(qtquick3d, QSSG_draw, int vertexCount, int instanceCount);

/*!
    \class QSSGRhiContext
    \inmodule QtQuick3D
    \since 6.7

    \brief QSSGRhiContext.
 */

/*!
    \class QSSGRhiGraphicsPipelineState
    \inmodule QtQuick3D
    \since 6.7

    \brief Graphics pipeline state for the spatial scene graph.

    This class is a convenience class used by QtQuick3D to wrap relevant pipeline state from the QRhi classes,
    like \l QRhiGraphicsPipeline. Most of the types and value used in QSSGRhiGraphicsPipelineState will
    therefore map directly to an equivalent QRhi type or class.
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::samples

    The sample count.

    \note A sample count of 1 means no multisample antialiasing.

    \sa QRhiSwapChain::sampleCount()
 */

/*!
    \enum QSSGRhiGraphicsPipelineState::Flag
    \value DepthTestEnabled
    \value DepthWriteEnabled
    \value BlendEnabled
    \value UsesStencilRef
    \value UsesScissor
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::depthFunc

    The depth comparison function.

    \sa QRhiGraphicsPipeline::CompareOp
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::cullMode

    Specifies the culling mode.

    \sa QRhiGraphicsPipeline::CullMode
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::stencilOpFrontState

    Describes the stencil operation state.

    \sa QRhiGraphicsPipeline::StencilOpState
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::stencilWriteMask

    The stencil write mask value. The default value is \c 0xFF.

    \sa QRhiGraphicsPipeline::stencilWriteMask()
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::stencilRef

    The active stencil reference value.

    \note Only used when \l{QSSGRhiGraphicsPipelineState::Flag::}{UsesStencilRef} is set.

    \sa QRhiCommandBuffer::setStencilRef()
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::depthBias

    The depth bias. The default value is 0.

    \sa QRhiGraphicsPipeline::depthBias()
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::slopeScaledDepthBias

    The slope scaled depth bias. The default value is 0.

    \sa QRhiGraphicsPipeline::slopeScaledDepthBias()
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::targetBlend

    The blend state for one color attachment.

    \sa QRhiGraphicsPipeline::TargetBlend
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::colorAttachmentCount

    The number of color attachments. The default is 1.

    \sa QRhiTextureRenderTargetDescription::setColorAttachments(),
        QRhiTextureRenderTargetDescription::colorAttachmentCount()
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::viewport

    The viewport dimensions used for rendering.
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::scissor

    The scissor rect.

    \note Only used if \l{QSSGRhiGraphicsPipelineState::Flag::}{UsesScissor} is set.

    \sa QRhiCommandBuffer::setScissor()
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::lineWidth

    The line width used. The default is 1.0

    \note For values other than 1.0 it's required that feature \l QRhi::WideLines is reported
    as supported at runtime.
 */

/*!
    \variable QSSGRhiGraphicsPipelineState::polygonMode

     The polygon mode value. The default is \l{QRhiGraphicsPipeline::Fill}{Fill}.

     \sa QRhiGraphicsPipeline::polygonMode()
 */

/*!
    \struct QSSGRhiSamplerDescription
    \inmodule QtQuick3D
    \since 6.7

    \brief QSSGRhiSamplerDescription.

    Convenience class used to request a \l QRhiSampler from QtQuick3D internal cache.

    \note Samplers are owned by QtQuick3D.

    \sa QSSGRhiContext::sampler()
*/

/*!
    \variable QSSGRhiSamplerDescription::minFilter

    The minification filter mode.

    \sa QRhiSampler::Filter, QRhiSampler::minFilter()
 */

/*!
    \variable QSSGRhiSamplerDescription::magFilter

    The magnification filter mode.

    \sa QRhiSampler::Filter, QRhiSampler::magFilter()
 */

/*!
    \variable QSSGRhiSamplerDescription::mipmap

    The mipmap filtering mode.

    \sa QRhiSampler::Filter, QRhiSampler::mipmapMode()
 */

/*!
    \variable QSSGRhiSamplerDescription::hTiling

    The horizontal wrap mode.

    \sa QRhiSampler::AddressMode
 */

/*!
    \variable QSSGRhiSamplerDescription::vTiling

    The vertical wrap mode.

    \sa QRhiSampler::AddressMode
 */

/*!
    \variable QSSGRhiSamplerDescription::zTiling

    The depth wrap mode.

    \sa QRhiSampler::AddressMode
 */

QSSGRhiBuffer::QSSGRhiBuffer(QSSGRhiContext &context,
                             QRhiBuffer::Type type,
                             QRhiBuffer::UsageFlags usageMask,
                             quint32 stride,
                             qsizetype size,
                             QRhiCommandBuffer::IndexFormat indexFormat)
    : m_context(context),
      m_stride(stride),
      m_indexFormat(indexFormat)
{
    QSSG_ASSERT(size >= 0, size = 0);
    m_buffer = m_context.rhi()->newBuffer(type, usageMask, quint32(size));
    if (!m_buffer->create())
        qWarning("Failed to build QRhiBuffer with size %d", m_buffer->size());
}

QSSGRhiBuffer::~QSSGRhiBuffer()
{
    delete m_buffer;
}

namespace QSSGRhiHelpers {
QRhiVertexInputAttribute::Format toVertexInputFormat(QSSGRenderComponentType compType, quint32 numComps)
{
    if (compType == QSSGRenderComponentType::Float32) {
        switch (numComps) {
        case 1:
            return QRhiVertexInputAttribute::Float;
        case 2:
            return QRhiVertexInputAttribute::Float2;
        case 3:
            return QRhiVertexInputAttribute::Float3;
        case 4:
            return QRhiVertexInputAttribute::Float4;
        default:
            break;
        }
    } else if (compType == QSSGRenderComponentType::UnsignedInt32) {
        switch (numComps) {
        case 1:
            return QRhiVertexInputAttribute::UInt;
        case 2:
            return QRhiVertexInputAttribute::UInt2;
        case 3:
            return QRhiVertexInputAttribute::UInt3;
        case 4:
            return QRhiVertexInputAttribute::UInt4;
        default:
            break;
        }
    } else if (compType == QSSGRenderComponentType::Int32) {
        switch (numComps) {
        case 1:
            return QRhiVertexInputAttribute::SInt;
        case 2:
            return QRhiVertexInputAttribute::SInt2;
        case 3:
            return QRhiVertexInputAttribute::SInt3;
        case 4:
            return QRhiVertexInputAttribute::SInt4;
        default:
            break;
        }
    }
    Q_ASSERT(false);
    return QRhiVertexInputAttribute::Float4;
}

QRhiGraphicsPipeline::Topology toTopology(QSSGRenderDrawMode drawMode)
{
    switch (drawMode) {
    case QSSGRenderDrawMode::Points:
        return QRhiGraphicsPipeline::Points;
    case QSSGRenderDrawMode::LineStrip:
        return QRhiGraphicsPipeline::LineStrip;
    case QSSGRenderDrawMode::Lines:
        return QRhiGraphicsPipeline::Lines;
    case QSSGRenderDrawMode::TriangleStrip:
        return QRhiGraphicsPipeline::TriangleStrip;
    case QSSGRenderDrawMode::TriangleFan:
        return QRhiGraphicsPipeline::TriangleFan;
    case QSSGRenderDrawMode::Triangles:
        return QRhiGraphicsPipeline::Triangles;
    case QSSGRenderDrawMode::LineLoop:
        QSSG_ASSERT_X(false, "LineLoop draw mode is not supported", return QRhiGraphicsPipeline::Triangles);
    }

    Q_UNREACHABLE_RETURN(QRhiGraphicsPipeline::Triangles);
}

void bakeVertexInputLocations(QSSGRhiInputAssemblerState *ia, const QSSGRhiShaderPipeline &shaders, int instanceBufferBinding)
{
    if (!shaders.vertexStage())
        return;

    const auto &vertexInputs = shaders.vertexInputs();

    QVarLengthArray<QRhiVertexInputAttribute, 8> attrs;
    int inputIndex = 0;
    for (auto it = ia->inputLayout.cbeginAttributes(), itEnd = ia->inputLayout.cendAttributes(); it != itEnd; ++it) {
        const QSSGRhiInputAssemblerState::InputSemantic sem = ia->inputs.at(inputIndex); // avoid detaching - submeshes share the same name list
        auto vertexInputVar = vertexInputs.constFind(sem);
        if (vertexInputVar != vertexInputs.constEnd()) {
            attrs.append(*it);
            attrs.last().setLocation(vertexInputVar->location);
        } // else the mesh has an input attribute that is not declared and used in the vertex shader - that's fine

        ++inputIndex;
    }

    // Add instance buffer input if necessary
    if (instanceBufferBinding > 0) {
        auto instanceBufferLocations = shaders.instanceBufferLocations();
        // transform0
        attrs.append(QRhiVertexInputAttribute(instanceBufferBinding,
                                             instanceBufferLocations.transform0,
                                             QRhiVertexInputAttribute::Float4,
                                             0));
        // transform1
        attrs.append(QRhiVertexInputAttribute(instanceBufferBinding,
                                             instanceBufferLocations.transform1,
                                             QRhiVertexInputAttribute::Float4,
                                             sizeof(float) * 4));
        // transform2
        attrs.append(QRhiVertexInputAttribute(instanceBufferBinding,
                                             instanceBufferLocations.transform2,
                                             QRhiVertexInputAttribute::Float4,
                                             sizeof(float) * 4 * 2));
        // color
        attrs.append(QRhiVertexInputAttribute(instanceBufferBinding,
                                             instanceBufferLocations.color,
                                             QRhiVertexInputAttribute::Float4,
                                             sizeof(float) * 4 * 3));
        // data
        attrs.append(QRhiVertexInputAttribute(instanceBufferBinding,
                                             instanceBufferLocations.data,
                                             QRhiVertexInputAttribute::Float4,
                                             sizeof(float) * 4 * 4));
    }

    ia->inputLayout.setAttributes(attrs.cbegin(), attrs.cend());
}

} // namespace QSSGRhiHelpers

void QSSGRhiShaderPipeline::addStage(const QRhiShaderStage &stage, StageFlags flags)
{
    m_stages.append(stage);

    // Copy all member infos for the uniform block with binding 0 into m_ub0
    // for faster lookup.
    if (stage.type() == QRhiShaderStage::Vertex) {
        // Optimize by doing it only for the vertex shader. This code path is
        // only hit for pipelines with vertex+fragment stages and an in shaders
        // from materials an identical uniform block is present in both
        // shaders, so go through only one of them.
        const QVector<QShaderDescription::UniformBlock> uniformBlocks = stage.shader().description().uniformBlocks();
        for (const QShaderDescription::UniformBlock &blk : uniformBlocks) {
            if (blk.binding == 0) {
                m_ub0Size = blk.size;
                m_ub0NextUBufOffset = m_context.rhi()->ubufAligned(m_ub0Size);
                for (const QShaderDescription::BlockVariable &var : blk.members)
                    m_ub0[var.name] = var;
                break;
            }
        }
        // Now the same for vertex inputs.
        if (!flags.testFlag(UsedWithoutIa)) {
            const QVector<QShaderDescription::InOutVariable> inputs = stage.shader().description().inputVariables();
            for (const QShaderDescription::InOutVariable &var : inputs) {
                if (var.name == QSSGMesh::MeshInternal::getPositionAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::PositionSemantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getNormalAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::NormalSemantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getUV0AttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::TexCoord0Semantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getUV1AttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::TexCoord1Semantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getLightmapUVAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::TexCoordLightmapSemantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getTexTanAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::TangentSemantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getTexBinormalAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::BinormalSemantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getColorAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::ColorSemantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getJointAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::JointSemantic] = var;
                } else if (var.name == QSSGMesh::MeshInternal::getWeightAttrName()) {
                    m_vertexInputs[QSSGRhiInputAssemblerState::WeightSemantic] = var;
                } else if (var.name == "qt_instanceTransform0") {
                    instanceLocations.transform0 = var.location;
                } else if (var.name == "qt_instanceTransform1") {
                    instanceLocations.transform1 = var.location;
                } else if (var.name == "qt_instanceTransform2") {
                    instanceLocations.transform2 = var.location;
                } else if (var.name == "qt_instanceColor") {
                    instanceLocations.color = var.location;
                } else if (var.name == "qt_instanceData") {
                    instanceLocations.data = var.location;
                } else {
                    qWarning("Ignoring vertex input %s in shader", var.name.constData());
                }
            }
        }
    }

    const QVector<QShaderDescription::InOutVariable> combinedImageSamplers  = stage.shader().description().combinedImageSamplers();
    for (const QShaderDescription::InOutVariable &var : combinedImageSamplers)
        m_combinedImageSamplers[var.name] = var;

    const QVector<QShaderDescription::InOutVariable> storageImages  = stage.shader().description().storageImages() + stage.shader().description().separateImages();
    for (const QShaderDescription::InOutVariable &var : storageImages)
        m_storageImages[var.name] = var;

    std::fill(m_materialImageSamplerBindings,
              m_materialImageSamplerBindings + size_t(QSSGRhiSamplerBindingHints::BindingMapSize),
              -1);
}

int QSSGRhiShaderPipeline::ub0DirectionalLightDataOffset() const
{
    return m_ub0NextUBufOffset + m_context.rhi()->ubufAligned(sizeof(QSSGShaderLightsUniformData));
}

void QSSGRhiShaderPipeline::setUniformValue(char *ubufData, const char *name, const QVariant &inValue, QSSGRenderShaderValue::Type inType)
{
    using namespace QSSGRenderShaderValue;
    switch (inType) {
    case QSSGRenderShaderValue::Integer:
    {
        const qint32 v = inValue.toInt();
        setUniform(ubufData, name, &v, sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::IntegerVec2:
    {
        const ivec2 v = inValue.value<ivec2>();
        setUniform(ubufData, name, &v, 2 * sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::IntegerVec3:
    {
        const ivec3 v = inValue.value<ivec3>();
        setUniform(ubufData, name, &v, 3 * sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::IntegerVec4:
    {
        const ivec4 v = inValue.value<ivec4>();
        setUniform(ubufData, name, &v, 4 * sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::Boolean:
    {
        // whatever bool is does not matter, what matters is that the GLSL bool is 4 bytes
        const qint32 v = inValue.value<bool>();
        setUniform(ubufData, name, &v, sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::BooleanVec2:
    {
        const bvec2 b = inValue.value<bvec2>();
        const ivec2 v(b.x, b.y);
        setUniform(ubufData, name, &v, 2 * sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::BooleanVec3:
    {
        const bvec3 b = inValue.value<bvec3>();
        const ivec3 v(b.x, b.y, b.z);
        setUniform(ubufData, name, &v, 3 * sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::BooleanVec4:
    {
        const bvec4 b = inValue.value<bvec4>();
        const ivec4 v(b.x, b.y, b.z, b.w);
        setUniform(ubufData, name, &v, 4 * sizeof(qint32));
    }
        break;
    case QSSGRenderShaderValue::Float:
    {
        const float v = inValue.value<float>();
        setUniform(ubufData, name, &v, sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Vec2:
    {
        const QVector2D v = inValue.value<QVector2D>();
        setUniform(ubufData, name, &v, 2 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Vec3:
    {
        const QVector3D v = inValue.value<QVector3D>();
        setUniform(ubufData, name, &v, 3 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Vec4:
    {
        const QVector4D v = inValue.value<QVector4D>();
        setUniform(ubufData, name, &v, 4 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Rgba:
    {
        const QVector4D v = QSSGUtils::color::sRGBToLinear(inValue.value<QColor>());
        setUniform(ubufData, name, &v, 4 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::UnsignedInteger:
    {
        const quint32 v = inValue.value<quint32>();
        setUniform(ubufData, name, &v, sizeof(quint32));
    }
        break;
    case QSSGRenderShaderValue::UnsignedIntegerVec2:
    {
        const uvec2 v = inValue.value<uvec2>();
        setUniform(ubufData, name, &v, 2 * sizeof(quint32));
    }
        break;
    case QSSGRenderShaderValue::UnsignedIntegerVec3:
    {
        const uvec3 v = inValue.value<uvec3>();
        setUniform(ubufData, name, &v, 3 * sizeof(quint32));
    }
        break;
    case QSSGRenderShaderValue::UnsignedIntegerVec4:
    {
        const uvec4 v = inValue.value<uvec4>();
        setUniform(ubufData, name, &v, 4 * sizeof(quint32));
    }
        break;
    case QSSGRenderShaderValue::Matrix3x3:
    {
        const QMatrix3x3 m = inValue.value<QMatrix3x3>();
        setUniform(ubufData, name, m.constData(), 12 * sizeof(float), nullptr, QSSGRhiShaderPipeline::UniformFlag::Mat3);
    }
        break;
    case QSSGRenderShaderValue::Matrix4x4:
    {
        const QMatrix4x4 v = inValue.value<QMatrix4x4>();
        setUniform(ubufData, name, v.constData(), 16 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Size:
    {
        const QSize s = inValue.value<QSize>();
        float v[2] = { float(s.width()), float(s.height()) };
        setUniform(ubufData, name, v, 2 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::SizeF:
    {
        const QSizeF s = inValue.value<QSizeF>();
        float v[2] = { float(s.width()), float(s.height()) };
        setUniform(ubufData, name, v, 2 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Point:
    {
        const QPoint p = inValue.value<QPoint>();
        float v[2] = { float(p.x()), float(p.y()) };
        setUniform(ubufData, name, v, 2 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::PointF:
    {
        const QPointF p = inValue.value<QPointF>();
        float v[2] = { float(p.x()), float(p.y()) };
        setUniform(ubufData, name, v, 2 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Rect:
    {
        const QRect r = inValue.value<QRect>();
        float v[4] = { float(r.x()), float(r.y()), float(r.width()), float(r.height()) };
        setUniform(ubufData, name, v, 4 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::RectF:
    {
        const QRectF r = inValue.value<QRectF>();
        float v[4] = { float(r.x()), float(r.y()), float(r.width()), float(r.height()) };
        setUniform(ubufData, name, v, 4 * sizeof(float));
    }
        break;
    case QSSGRenderShaderValue::Quaternion:
    {
        const QQuaternion q = inValue.value<QQuaternion>();
        float v[4] = { float(q.x()), float(q.y()), float(q.z()), float(q.scalar()) };
        setUniform(ubufData, name, v, 4 * sizeof(float));
    }
        break;
    default:
        qWarning("Attempted to set uniform %s value with unsupported data type %i",
                 name, int(inType));
        break;
    }
}

int QSSGRhiShaderPipeline::offsetOfUniform(const QByteArray &name)
{
    auto iter = m_ub0.constFind(name);
    if (iter != m_ub0.cend())
        return iter->offset;
    return -1;
}

static QString getUBMemberSizeWarning(QLatin1StringView name, qsizetype correctedSize, qsizetype requestedSize)
{
    return QStringLiteral("Uniform block member '%1' got %2 bytes whereas the true size is %3").arg(name, QString::number(correctedSize), QString::number(requestedSize));
}

void QSSGRhiShaderPipeline::setUniform(char *ubufData, const char *name, const void *data, size_t size, int *storeIndex, UniformFlags flags)
{
    int index = -1;
    if (!storeIndex || *storeIndex == -1) {
        const QByteArray ba = QByteArray::fromRawData(name, strlen(name));
        auto it = m_uniformIndex.constFind(ba);
        if (it != m_uniformIndex.cend()) {
            index = int(*it);
        } else if (ba.size() < qsizetype(sizeof(QSSGRhiShaderUniform::name))) {
            QSSGRhiShaderUniform u;
            memcpy(u.name, name, ba.size() + 1);
            u.size = size;

            const int new_idx = m_uniforms.size();
            m_uniformIndex[name] = new_idx; // key is name, not ba, this has to be a deep copy QByteArray
            m_uniforms.push_back(u);
            index = new_idx;
        } else {
            qWarning("Attempted to set uniform with too long name: %s", name);
            return;
        }
        if (storeIndex)
            *storeIndex = index;
    } else {
        index = *storeIndex;
    }

    Q_ASSERT(index >= 0);
    QSSGRhiShaderUniform &u = m_uniforms[index];
    if (size <= u.size) {
        if (u.offset == SIZE_MAX && u.maybeExists) {
            auto it = m_ub0.constFind(QByteArray::fromRawData(u.name, strlen(u.name)));
            if (it != m_ub0.constEnd()) {
                u.offset = it->offset;
                QSSG_ASSERT_X(QSSG_DEBUG_COND(int(u.size) == it->size), qPrintable(getUBMemberSizeWarning(QLatin1StringView(it->name), u.size, it->size)), return);
            }
        }
        if (u.offset == SIZE_MAX) {
            // must silently ignore uniforms that are not in the actual shader
            u.maybeExists = false; // but do not try again
            return;
        }

        char *dst = ubufData + u.offset;
        if (flags.testFlag(UniformFlag::Mat3)) {
            // mat3 is still 4 floats per column in the uniform buffer (but there
            // is no 4th column), so 48 bytes altogether, not 36 or 64.
            const float *src = static_cast<const float *>(data);
            memcpy(dst, src, 3 * sizeof(float));
            memcpy(dst + 4 * sizeof(float), src + 3, 3 * sizeof(float));
            memcpy(dst + 8 * sizeof(float), src + 6, 3 * sizeof(float));
        } else {
            memcpy(dst, data, size);
        }
    } else {
        qWarning("Attempted to set %u bytes to uniform %s with size %u", uint(size), name, uint(u.size));
    }
}

// Quick3D uniform buffer is std140 type and all array data should be stored in this rule.
// You can check it in glspec45.core.pdf's 7.6.2.2.(4)
// https://www.khronos.org/registry/OpenGL/specs/gl/glspec45.core.pdf
void QSSGRhiShaderPipeline::setUniformArray(char *ubufData, const char *name, const void *data, size_t itemCount, QSSGRenderShaderValue::Type type, int *storeIndex)
{
    using namespace QSSGRenderShaderValue;

    QSSGRhiShaderUniformArray *ua = nullptr;
    constexpr size_t std140BaseTypeSize = 4 * sizeof(float);

    static const auto checkSize = [std140BaseTypeSize](QSSGRhiShaderUniformArray *ua) -> bool {
        Q_UNUSED(std140BaseTypeSize); // Silence clang warning about unneeded lambda capture (MSVC requires it be captrued).
        const size_t uniformSize = std140BaseTypeSize < ua->typeSize ? ua->typeSize * ua->itemCount : std140BaseTypeSize * ua->itemCount;
        QSSG_ASSERT_X(uniformSize == ua->size, qPrintable(getUBMemberSizeWarning(QLatin1StringView(ua->name), uniformSize, ua->size)), return false);
        return true;
    };

    if (!storeIndex || *storeIndex == -1) {
        int index = -1;
        const QByteArray ba = QByteArray::fromRawData(name, strlen(name));
        auto it = m_uniformIndex.constFind(ba);
        if (it != m_uniformIndex.cend()) {
            index = int(*it);
            ua = &m_uniformArrays[index];
        } else if (ba.size() < qsizetype(sizeof(QSSGRhiShaderUniformArray::name))) {
            index = m_uniformArrays.size();
            m_uniformArrays.push_back(QSSGRhiShaderUniformArray());
            m_uniformIndex[name] = index; // key needs deep copy
            ua = &m_uniformArrays.last();
            memcpy(ua->name, name, ba.size() + 1);
        } else {
            qWarning("Attempted to set uniform array with too long name: %s", name);
            return;
        }
        if (storeIndex)
            *storeIndex = index;
    } else {
        ua = &m_uniformArrays[*storeIndex];
    }

    if (!ua)
        return;

    if (ua->offset == SIZE_MAX && ua->maybeExists) {
        auto it = m_ub0.constFind(QByteArray::fromRawData(ua->name, strlen(ua->name)));
        if (it != m_ub0.constEnd()) {
            ua->offset = it->offset;
            ua->size = it->size;
        }
    }
    if (ua->offset == SIZE_MAX) {
        // must silently ignore uniforms that are not in the actual shader
        ua->maybeExists = false; // but do not try again
        return;
    }

    char *p = ubufData + ua->offset;

    switch (type) {
    case QSSGRenderShaderValue::Integer:
    {
        const qint32 *v = static_cast<const qint32 *>(data);
        if (sizeof(qint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = sizeof(qint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::IntegerVec2:
    {
        const ivec2 *v = static_cast<const ivec2 *>(data);
        if (2 * sizeof(qint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 2 * sizeof(qint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::IntegerVec3:
    {
        const QSSGRenderShaderValue::ivec3 *v = static_cast<const QSSGRenderShaderValue::ivec3 *>(data);
        if (3 * sizeof(qint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 3 * sizeof(qint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::IntegerVec4:
    {
        const ivec4 *v = static_cast<const ivec4 *>(data);
        if (4 * sizeof(qint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 4 * sizeof(qint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        memcpy(p, v, ua->typeSize * ua->itemCount);
    }
        break;
    case QSSGRenderShaderValue::Float:
    {
        const float *v = static_cast<const float *>(data);
        if (sizeof(float) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = sizeof(float);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::Vec2:
    {
        const QVector2D *v = static_cast<const QVector2D *>(data);
        if (2 * sizeof(float) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 2 * sizeof(float);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::Vec3:
    {
        const QVector3D *v = static_cast<const QVector3D *>(data);
        if (3 * sizeof(float) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 3 * sizeof(float);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::Vec4:
    {
        const QVector4D *v = static_cast<const QVector4D *>(data);
        if (4 * sizeof(float) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 4 * sizeof(float);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        memcpy(p, v, ua->typeSize * ua->itemCount);
    }
        break;
    case QSSGRenderShaderValue::Rgba:
    {
        const QColor *v = static_cast<const QColor *>(data);
        if (4 * sizeof(float) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 4 * sizeof(float);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i) {
            const QVector4D vi = QSSGUtils::color::sRGBToLinear(v[i]);
            memcpy(p + i * std140BaseTypeSize, &vi, ua->typeSize);
        }
    }
        break;
    case QSSGRenderShaderValue::UnsignedInteger:
    {
        const quint32 *v = static_cast<const quint32 *>(data);
        if (sizeof(quint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = sizeof(quint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::UnsignedIntegerVec2:
    {
        const uvec2 *v = static_cast<const uvec2 *>(data);
        if (2 * sizeof(quint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 2 * sizeof(quint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::UnsignedIntegerVec3:
    {
        const uvec3 *v = static_cast<const uvec3 *>(data);
        if (3 * sizeof(quint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 3 * sizeof(quint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (size_t i = 0; i < itemCount; ++i)
            memcpy(p + i * std140BaseTypeSize, &v[i], ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::UnsignedIntegerVec4:
    {
        const uvec4 *v = static_cast<const uvec4 *>(data);
        if (4 * sizeof(quint32) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 4 * sizeof(quint32);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        memcpy(p, v, ua->typeSize * ua->itemCount);
    }
        break;
    case QSSGRenderShaderValue::Matrix3x3:
    {
        const QMatrix3x3 *v = static_cast<const QMatrix3x3 *>(data);
        if (12 * sizeof(float) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 12 * sizeof(float);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (uint i = 0; i < ua->itemCount; ++i) {
            memcpy(p + i * ua->typeSize, v[i].constData(), 3 * sizeof(float));
            memcpy(p + i * ua->typeSize + 4 * sizeof(float), v[i].constData() + 3, 3 * sizeof(float));
            memcpy(p + i * ua->typeSize + 8 * sizeof(float), v[i].constData() + 6, 3 * sizeof(float));
        }
    }
        break;
    case QSSGRenderShaderValue::Matrix4x4:
    {
        const QMatrix4x4 *v = static_cast<const QMatrix4x4 *>(data);
        if (16 * sizeof(float) != ua->typeSize || itemCount != ua->itemCount) {
            ua->typeSize = 16 * sizeof(float);
            ua->itemCount = itemCount;
        }

        QSSG_ASSERT(QSSG_DEBUG_COND(checkSize(ua)), return);

        for (uint i = 0; i < ua->itemCount; ++i)
            memcpy(p + i * ua->typeSize, &v[i] , ua->typeSize);
    }
        break;
    case QSSGRenderShaderValue::Boolean:
    case QSSGRenderShaderValue::BooleanVec2:
    case QSSGRenderShaderValue::BooleanVec3:
    case QSSGRenderShaderValue::BooleanVec4:
    case QSSGRenderShaderValue::Size:
    case QSSGRenderShaderValue::SizeF:
    case QSSGRenderShaderValue::Point:
    case QSSGRenderShaderValue::PointF:
    case QSSGRenderShaderValue::Rect:
    case QSSGRenderShaderValue::RectF:
    case QSSGRenderShaderValue::Quaternion:
    default:
        qWarning("Attempted to set uniform %s value with type %d that is unsupported for uniform arrays",
                 name, int(type));
        break;
    }
}

void QSSGRhiShaderPipeline::ensureCombinedUniformBuffer(QRhiBuffer **ubuf)
{
    const quint32 alignedLightsSize = m_context.rhi()->ubufAligned(sizeof(QSSGShaderLightsUniformData));
    const quint32 alignedDirectionalLightsSize = m_context.rhi()->ubufAligned(sizeof(QSSGShaderDirectionalLightsUniformData));
    const quint32 totalBufferSize = m_ub0NextUBufOffset + alignedLightsSize + alignedDirectionalLightsSize;
    if (!*ubuf) {
        *ubuf = m_context.rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, totalBufferSize);
        (*ubuf)->create();
    }
    if ((*ubuf)->size() < totalBufferSize) {
        (*ubuf)->setSize(totalBufferSize);
        (*ubuf)->create();
    }
}

void QSSGRhiShaderPipeline::ensureUniformBuffer(QRhiBuffer **ubuf)
{
    if (!*ubuf) {
        *ubuf = m_context.rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, m_ub0Size);
        (*ubuf)->create();
    }
}

int QSSGRhiShaderPipeline::bindingForTexture(const char *name, int hint)
{
    if (hint >= 0) {
        const int binding = m_materialImageSamplerBindings[hint];
        if (binding >= 0)
            return binding;
    }

    auto it = m_combinedImageSamplers.constFind(QByteArray::fromRawData(name, strlen(name)));
    const int binding = it != m_combinedImageSamplers.cend() ? it->binding : -1;
    if (hint >= 0)
        m_materialImageSamplerBindings[hint] = binding;

    return binding;
}

int QSSGRhiShaderPipeline::bindingForImage(const char *name)
{
    auto it = m_storageImages.constFind(QByteArray::fromRawData(name, strlen(name)));
    const int binding = it != m_storageImages.cend() ? it->binding : -1;
    return binding;
}

void QSSGRhiShaderPipeline::setShaderResources(char *ubufData,
                                               QSSGBufferManager &theBufferManager,
                                               const QByteArray &inPropertyName,
                                               const QVariant &propertyValue,
                                               QSSGRenderShaderValue::Type inPropertyType)
{
    if (inPropertyType == QSSGRenderShaderValue::Texture) {
        QSSGRenderImage *image = propertyValue.value<QSSGRenderImage *>();
        if (image) {
            const QSSGRenderImageTexture texture = theBufferManager.loadRenderImage(image);
            if (texture.m_texture) {
                const QSSGRhiTexture t = {
                    inPropertyName,
                    texture.m_texture,
                    { QSSGRhiHelpers::toRhi(image->m_minFilterType),
                            QSSGRhiHelpers::toRhi(image->m_magFilterType),
                            image->m_mipFilterType != QSSGRenderTextureFilterOp::None ? QSSGRhiHelpers::toRhi(image->m_mipFilterType) : QRhiSampler::None,
                            QSSGRhiHelpers::toRhi(image->m_horizontalTilingMode),
                            QSSGRhiHelpers::toRhi(image->m_verticalTilingMode),
                            QSSGRhiHelpers::toRhi(image->m_depthTilingMode)
                    }
                };
                addExtraTexture(t);
            }
        }
    } else {
        setUniformValue(ubufData, inPropertyName, propertyValue, inPropertyType);
    }
}


/*!
    \internal
 */
QSSGRhiContext::QSSGRhiContext(QRhi *rhi)
    : d_ptr(new QSSGRhiContextPrivate(*this, rhi))
{
    Q_ASSERT(rhi);
    Q_STATIC_ASSERT(int(QSSGRhiSamplerBindingHints::LightProbe) > int(QSSGRenderableImage::Type::Occlusion));
}

/*!
    \internal
 */
QSSGRhiContext::~QSSGRhiContext()
{
    Q_D(QSSGRhiContext);
    d->releaseCachedResources();

    qDeleteAll(d->m_textures);
    qDeleteAll(d->m_meshes);
}

/*!
    \return The QRhi object used by the Qt Quick 3D renderer.
 */
QRhi *QSSGRhiContext::rhi() const
{
    Q_D(const QSSGRhiContext);
    return d->m_rhi;
}

/*!
    \return true if the renderer is initialized successfully.
 */
bool QSSGRhiContext::isValid() const
{
    Q_D(const QSSGRhiContext);
    return d->m_rhi != nullptr;
}

void QSSGRhiContextPrivate::setMainRenderPassDescriptor(QRhiRenderPassDescriptor *rpDesc)
{
    m_mainRpDesc = rpDesc;
}

/*!
    \return The QRhiRenderPassDescriptor used by the main render pass of the Qt
    Quick 3D renderer.
 */
QRhiRenderPassDescriptor *QSSGRhiContext::mainRenderPassDescriptor() const
{
    Q_D(const QSSGRhiContext);
    return d->m_mainRpDesc;
}

void QSSGRhiContextPrivate::setCommandBuffer(QRhiCommandBuffer *cb)
{
    m_cb = cb;
}

/*!
    \return The current frame's command buffer used by the Qt Quick 3D renderer.
 */
QRhiCommandBuffer *QSSGRhiContext::commandBuffer() const
{
    Q_D(const QSSGRhiContext);
    return d->m_cb;
}

void QSSGRhiContextPrivate::setRenderTarget(QRhiRenderTarget *rt)
{
    m_rt = rt;
}

/*!
    \return The render target the Qt Quick 3D renderer uses for its main render
    pass in the current frame.

    This can effectively be a render target from a swapchain, if the \l View3D
    uses a renderMode other than Offscreen. More commonly, the render target
    refers to a texture (i.e., is a QRhiTextureRenderTarget), e.g. because the
    renderMode is the default Offscreen, or because post-processing effects are
    in use.
 */
QRhiRenderTarget *QSSGRhiContext::renderTarget() const
{
    Q_D(const QSSGRhiContext);
    return d->m_rt;
}

void QSSGRhiContextPrivate::setMainPassSampleCount(int samples)
{
    m_mainSamples = samples;
}

/*!
    Returns the sample count used in the main render pass.
 */
int QSSGRhiContext::mainPassSampleCount() const
{
    Q_D(const QSSGRhiContext);
    return d->m_mainSamples;
}

void QSSGRhiContextPrivate::setMainPassViewCount(int viewCount)
{
    m_mainViewCount = viewCount;
}

/*!
    Returns the multiview count used in the main render pass. This is either 2,
    when multiview rendering is in use, or 1 (no multiview).
 */
int QSSGRhiContext::mainPassViewCount() const
{
    Q_D(const QSSGRhiContext);
    return d->m_mainViewCount;
}

void QSSGRhiContextPrivate::releaseCachedResources()
{
    for (QSSGRhiDrawCallData &dcd : m_drawCallData) {
        // We don't call releaseDrawCallData() here, since we're anyways
        // are going to delete the non-owned resources further down and
        // there's no point in removing each of those one-by-one, as is
        // the case with releaseDrawCallData().
        // This speeds up the release of cached resources greatly when there
        // are many entries in the map, also at application shutdown.
        dcd.reset();
    }

    m_drawCallData.clear();

    qDeleteAll(m_pipelines);
    qDeleteAll(m_computePipelines);
    qDeleteAll(m_srbCache);
    qDeleteAll(m_dummyTextures);

    m_pipelines.clear();
    m_computePipelines.clear();
    m_srbCache.clear();
    m_dummyTextures.clear();

    for (const auto &samplerInfo : std::as_const(m_samplers))
        delete samplerInfo.second;

    m_samplers.clear();

    for (const auto &particleData : std::as_const(m_particleData))
        delete particleData.texture;

    m_particleData.clear();

    for (const auto &instanceData : std::as_const(m_instanceBuffers)) {
        if (instanceData.owned)
            delete instanceData.buffer;
    }

    m_instanceBuffers.clear();

    for (const auto &instanceData : std::as_const(m_instanceBuffersLod)) {
        if (instanceData.owned)
            delete instanceData.buffer;
    }

    m_instanceBuffersLod.clear();
}

QRhiShaderResourceBindings *QSSGRhiContextPrivate::srb(const QSSGRhiShaderResourceBindingList &bindings)
{
    auto it = m_srbCache.constFind(bindings);
    if (it != m_srbCache.constEnd())
        return *it;

    QRhiShaderResourceBindings *srb = m_rhi->newShaderResourceBindings();
    srb->setBindings(bindings.v, bindings.v + bindings.p);
    if (srb->create()) {
        m_srbCache.insert(bindings, srb);
    } else {
        qWarning("Failed to build srb");
        delete srb;
        srb = nullptr;
    }
    return srb;
}

void QSSGRhiContextPrivate::releaseCachedSrb(QSSGRhiShaderResourceBindingList &bindings)
{
    auto srb = m_srbCache.take(bindings);
    delete srb;
}

void QSSGRhiContextPrivate::releaseDrawCallData(QSSGRhiDrawCallData &dcd)
{
    delete dcd.ubuf;
    dcd.ubuf = nullptr;
    auto srb = m_srbCache.take(dcd.bindings);
    QSSG_CHECK(srb == dcd.srb);
    delete srb;
    dcd.srb = nullptr;
    dcd.pipeline = nullptr;
}

QRhiGraphicsPipeline *QSSGRhiContextPrivate::pipeline(const QSSGRhiGraphicsPipelineState &ps,
                                                      QRhiRenderPassDescriptor *rpDesc,
                                                      QRhiShaderResourceBindings *srb)
{
    return pipeline(QSSGGraphicsPipelineStateKey::create(ps, rpDesc, srb), rpDesc, srb);
}

QRhiComputePipeline *QSSGRhiContextPrivate::computePipeline(const QShader &shader,
                                                            QRhiShaderResourceBindings *srb)
{
    return computePipeline(QSSGComputePipelineStateKey::create(shader, srb), srb);
}

QSSGRhiDrawCallData &QSSGRhiContextPrivate::drawCallData(const QSSGRhiDrawCallDataKey &key)
{
    return m_drawCallData[key];
}

using SamplerInfo = QPair<QSSGRhiSamplerDescription, QRhiSampler*>;

/*!
    \return a sampler with the filter and tiling modes specified in \a samplerDescription.

    The generated QRhiSampler objects are cached and reused. Thus this is a
    convenient way to gain access to a QRhiSampler with the given settings,
    without having to create a new, dedicated object all the time.

    The ownership of the returned QRhiSampler stays with Qt Quick 3D.
 */
QRhiSampler *QSSGRhiContext::sampler(const QSSGRhiSamplerDescription &samplerDescription)
{
    Q_D(QSSGRhiContext);
    auto compareSampler = [samplerDescription](const SamplerInfo &info){ return info.first == samplerDescription; };
    auto &samplers = d->m_samplers;
    const auto found = std::find_if(samplers.cbegin(), samplers.cend(), compareSampler);
    if (found != samplers.cend())
        return found->second;

    QRhiSampler *newSampler = d->m_rhi->newSampler(samplerDescription.magFilter,
                                                   samplerDescription.minFilter,
                                                   samplerDescription.mipmap,
                                                   samplerDescription.hTiling,
                                                   samplerDescription.vTiling,
                                                   samplerDescription.zTiling);
    if (!newSampler->create()) {
        qWarning("Failed to build image sampler");
        delete newSampler;
        return nullptr;
    }
    samplers << SamplerInfo{samplerDescription, newSampler};
    return newSampler;
}

/*!
    Adjusts \a samplerDescription's tiling and filtering modes based on the
    pixel size of \a texture.

    In most cases, \a samplerDescription is not changed. With older, legacy 3D
    APIs in use, there is however a chance that tiling modes such as
    \l{QRhiSampler::Repeat} are not supported for textures with a
    non-power-of-two width or height.

    This convenience function helps creating robust applications that can still
    function even when features such as \l{QRhi::NPOTTextureRepeat} are not
    supported by an OpenGL ES 2.0 or WebGL 1 implementation at run time.
 */
void QSSGRhiContext::checkAndAdjustForNPoT(QRhiTexture *texture, QSSGRhiSamplerDescription *samplerDescription)
{
    Q_D(const QSSGRhiContext);
    if (samplerDescription->mipmap != QRhiSampler::None
        || samplerDescription->hTiling != QRhiSampler::ClampToEdge
        || samplerDescription->vTiling != QRhiSampler::ClampToEdge
        || samplerDescription->zTiling != QRhiSampler::ClampToEdge)
    {
        if (d->m_rhi->isFeatureSupported(QRhi::NPOTTextureRepeat))
            return;

        const QSize pixelSize = texture->pixelSize();
        const int w = qNextPowerOfTwo(pixelSize.width() - 1);
        const int h = qNextPowerOfTwo(pixelSize.height() - 1);
        if (w != pixelSize.width() || h != pixelSize.height()) {
            static bool warnShown = false;
            if (!warnShown) {
                warnShown = true;
                qWarning("Attempted to use an unsupported filtering or wrap mode, "
                         "this is likely due to lacking proper support for non-power-of-two textures on this platform.\n"
                         "If this is with WebGL, try updating the application to use QQuick3D::idealSurfaceFormat() in main() "
                         "in order to ensure WebGL 2 is used.");
            }
            samplerDescription->mipmap = QRhiSampler::None;
            samplerDescription->hTiling = QRhiSampler::ClampToEdge;
            samplerDescription->vTiling = QRhiSampler::ClampToEdge;
            samplerDescription->zTiling = QRhiSampler::ClampToEdge;
        }
    }
}

void QSSGRhiContextPrivate::registerTexture(QRhiTexture *texture)
{
    m_textures.insert(texture);
}

void QSSGRhiContextPrivate::releaseTexture(QRhiTexture *texture)
{
    m_textures.remove(texture);
    delete texture;
}

void QSSGRhiContextPrivate::registerMesh(QSSGRenderMesh *mesh)
{
    m_meshes.insert(mesh);
}

void QSSGRhiContextPrivate::releaseMesh(QSSGRenderMesh *mesh)
{
    if (mesh) {
        for (const auto &subset : std::as_const(mesh->subsets)) {
            if (subset.rhi.targetsTexture) {
                // If there is a morph targets texture, it should be the same for
                // all subsets, so just release and break
                releaseTexture(subset.rhi.targetsTexture);
                break;
            }
        }
    }
    m_meshes.remove(mesh);
    delete mesh;
}

void QSSGRhiContextPrivate::cleanupDrawCallData(const QSSGRenderModel *model)
{
    // Find all QSSGRhiUniformBufferSet that reference model
    // and delete them
    const void *modelNode = model;
    auto it = m_drawCallData.begin();
    while (it != m_drawCallData.end()) {
        if (it.key().model == modelNode) {
            releaseDrawCallData(*it);
            it = m_drawCallData.erase(it);
        } else {
            ++it;
        }
    }
}

/*!
    \return a texture that has the specified \a flags and pixel \a size.

    This is intended to efficiently gain access to a "dummy" texture filled
    with a given \a fillColor, and reused in various places in the rendering
    stack.

    \a rub must be a valid QRhiResourceUpdateBatch since this function will
    create a new texture and generate content for it, if a suitable cached
    object is not found. The necessary upload operations are then enqueued on
    this given update batch.

    When \a arraySize is 2 or more, a 2D texture array is returned.

    The ownership of the returned texture stays with Qt Quick 3D.
 */
QRhiTexture *QSSGRhiContext::dummyTexture(QRhiTexture::Flags flags, QRhiResourceUpdateBatch *rub,
                                          const QSize &size, const QColor &fillColor, int arraySize)
{
    Q_D(QSSGRhiContext);
    auto it = d->m_dummyTextures.constFind({flags, size, fillColor, arraySize});
    if (it != d->m_dummyTextures.constEnd())
        return *it;

    QRhiTexture *t = arraySize < 2
        ? d->m_rhi->newTexture(QRhiTexture::RGBA8, size, 1, flags)
        : d->m_rhi->newTextureArray(QRhiTexture::RGBA8, arraySize, size, 1, flags);
    if (t->create()) {
        QImage image(t->pixelSize(), QImage::Format_RGBA8888);
        image.fill(fillColor);
        rub->uploadTexture(t, image);
        for (int layer = 1; layer < arraySize; ++layer)
            rub->uploadTexture(t, QRhiTextureUploadDescription(QRhiTextureUploadEntry(layer, 0, QRhiTextureSubresourceUploadDescription(image))));
    } else {
        qWarning("Failed to build dummy texture");
    }

    d->m_dummyTextures.insert({flags, size, fillColor, arraySize}, t);
    return t;
}

QSSGRhiInstanceBufferData &QSSGRhiContextPrivate::instanceBufferData(QSSGRenderInstanceTable *instanceTable)
{
    return m_instanceBuffers[instanceTable];
}

void QSSGRhiContextPrivate::releaseInstanceBuffer(QSSGRenderInstanceTable *instanceTable)
{
    auto it = m_instanceBuffers.constFind(instanceTable);
    if (it != m_instanceBuffers.constEnd()) {
        it->buffer->destroy();
        m_instanceBuffers.erase(it);
    }
}

QSSGRhiInstanceBufferData &QSSGRhiContextPrivate::instanceBufferData(const QSSGRenderModel *model)
{
    return m_instanceBuffersLod[model];
}

QSSGRhiParticleData &QSSGRhiContextPrivate::particleData(const QSSGRenderGraphObject *particlesOrModel)
{
    return m_particleData[particlesOrModel];
}

void QSSGRhiContextStats::start(QSSGRenderLayer *layer)
{
    layerKey = layer;
    PerLayerInfo &info(perLayerInfo[layerKey]);
    info.renderPasses.clear();
    info.externalRenderPass = {};
    info.currentRenderPassIndex = -1;
}

void QSSGRhiContextStats::stop(QSSGRenderLayer *layer)
{
    if (rendererDebugEnabled()) {
        PerLayerInfo &info(perLayerInfo[layer]);
        const int rpCount = info.renderPasses.size();
        qDebug("%d render passes in 3D renderer %p", rpCount, layer);
        for (int i = 0; i < rpCount; ++i) {
            const RenderPassInfo &rp(info.renderPasses[i]);
            qDebug("Render pass %d: rt name='%s' target size %dx%d pixels",
                   i, rp.rtName.constData(), rp.pixelSize.width(), rp.pixelSize.height());
            printRenderPass(rp);
        }
        if (info.externalRenderPass.indexedDraws.callCount || info.externalRenderPass.instancedIndexedDraws.callCount
                || info.externalRenderPass.draws.callCount || info.externalRenderPass.instancedDraws.callCount)
        {
            qDebug("Within external render passes:");
            printRenderPass(info.externalRenderPass);
        }
    }

    // a new start() may preceed stop() for the previous View3D, must handle this gracefully
    if (layerKey == layer)
        layerKey = nullptr;

    // The data must stay valid until the next start() with the same key, the
    // QQuick3DRenderStats and DebugView may read it.
}

void QSSGRhiContextStats::cleanupLayerInfo(QSSGRenderLayer *layer)
{
    perLayerInfo.remove(layer);
    dynamicDataSources.remove(layer);
}

void QSSGRhiContextStats::beginRenderPass(QRhiTextureRenderTarget *rt)
{
    PerLayerInfo &info(perLayerInfo[layerKey]);
    Q_TRACE(QSSG_renderPass_entry, QString::fromUtf8(rt->name()));
    info.renderPasses.append({ rt->name(), rt->pixelSize(), {}, {}, {}, {} });
    info.currentRenderPassIndex = info.renderPasses.size() - 1;
}

void QSSGRhiContextStats::endRenderPass()
{
    Q_TRACE(QSSG_renderPass_exit);
    PerLayerInfo &info(perLayerInfo[layerKey]);
    info.currentRenderPassIndex = -1;
}

QSSGRhiContextStats &QSSGRhiContextStats::get(QSSGRhiContext &rhiCtx)
{
    return QSSGRhiContextPrivate::get(&rhiCtx)->m_stats;
}

const QSSGRhiContextStats &QSSGRhiContextStats::get(const QSSGRhiContext &rhiCtx)
{
    return QSSGRhiContextPrivate::get(&rhiCtx)->m_stats;
}

bool QSSGRhiContextStats::profilingEnabled()
{
    static bool enabled = Q_QUICK3D_PROFILING_ENABLED;
    return enabled;
}

bool QSSGRhiContextStats::rendererDebugEnabled()
{
    static bool enabled = qgetenv("QSG_RENDERER_DEBUG").contains(QByteArrayLiteral("render"));
    return enabled;
}

bool QSSGRhiContextStats::isEnabled() const
{
    return !dynamicDataSources.isEmpty() || profilingEnabled() || rendererDebugEnabled()
            || Q_TRACE_ENABLED(QSSG_draw);
}

void QSSGRhiContextStats::drawIndexed(quint32 indexCount, quint32 instanceCount)
{
    Q_TRACE(QSSG_drawIndexed, indexCount, instanceCount);
    PerLayerInfo &info(perLayerInfo[layerKey]);
    RenderPassInfo &rp(info.currentRenderPassIndex >= 0 ? info.renderPasses[info.currentRenderPassIndex] : info.externalRenderPass);
    if (instanceCount > 1) {
        rp.instancedIndexedDraws.callCount += 1;
        rp.instancedIndexedDraws.vertexOrIndexCount += indexCount;
        rp.instancedIndexedDraws.instanceCount += instanceCount;
    } else {
        rp.indexedDraws.callCount += 1;
        rp.indexedDraws.vertexOrIndexCount += indexCount;
    }
}

void QSSGRhiContextStats::draw(quint32 vertexCount, quint32 instanceCount)
{
    Q_TRACE(QSSG_draw, vertexCount, instanceCount);
    PerLayerInfo &info(perLayerInfo[layerKey]);
    RenderPassInfo &rp(info.currentRenderPassIndex >= 0 ? info.renderPasses[info.currentRenderPassIndex] : info.externalRenderPass);
    if (instanceCount > 1) {
        rp.instancedDraws.callCount += 1;
        rp.instancedDraws.vertexOrIndexCount += vertexCount;
        rp.instancedDraws.instanceCount += instanceCount;
    } else {
        rp.draws.callCount += 1;
        rp.draws.vertexOrIndexCount += vertexCount;
    }
}

void QSSGRhiContextStats::printRenderPass(const QSSGRhiContextStats::RenderPassInfo &rp)
{
    qDebug("%llu indexed draw calls with %llu indices in total, "
           "%llu non-indexed draw calls with %llu vertices in total",
           rp.indexedDraws.callCount, rp.indexedDraws.vertexOrIndexCount,
           rp.draws.callCount, rp.draws.vertexOrIndexCount);
    if (rp.instancedIndexedDraws.callCount || rp.instancedDraws.callCount) {
        qDebug("%llu instanced indexed draw calls with %llu indices and %llu instances in total, "
               "%llu instanced non-indexed draw calls with %llu indices and %llu instances in total",
               rp.instancedIndexedDraws.callCount, rp.instancedIndexedDraws.vertexOrIndexCount, rp.instancedIndexedDraws.instanceCount,
               rp.instancedDraws.callCount, rp.instancedDraws.vertexOrIndexCount, rp.instancedDraws.instanceCount);
    }
}

void QSSGRhiShaderResourceBindingList::addUniformBuffer(int binding, QRhiShaderResourceBinding::StageFlags stage, QRhiBuffer *buf, int offset, int size, bool hasDynamicOffset)
{
#ifdef QT_DEBUG
    if (p == MAX_SIZE) {
        qWarning("Out of shader resource bindings slots (max is %d)", MAX_SIZE);
        return;
    }
#endif
    QRhiShaderResourceBinding::Data *d = QRhiImplementation::shaderResourceBindingData(v[p++]);
    h ^= qintptr(buf);
    d->binding = binding;
    d->stage = stage;
    d->type = QRhiShaderResourceBinding::UniformBuffer;
    d->u.ubuf.buf = buf;
    d->u.ubuf.offset = offset;
    d->u.ubuf.maybeSize = size; // 0 = all
    d->u.ubuf.hasDynamicOffset = hasDynamicOffset;
}

void QSSGRhiShaderResourceBindingList::addStorageBuffer(int binding, QRhiShaderResourceBinding::StageFlags stage, QRhiBuffer *buf, int offset, int size, bool hasDynamicOffset)
{
#ifdef QT_DEBUG
    if (p == MAX_SIZE) {
        qWarning("Out of shader resource bindings slots (max is %d)", MAX_SIZE);
        return;
    }
#endif
    QRhiShaderResourceBinding::Data *d = QRhiImplementation::shaderResourceBindingData(v[p++]);
    h ^= qintptr(buf);
    d->binding = binding;
    d->stage = stage;
    d->type = QRhiShaderResourceBinding::BufferLoadStore;
    d->u.ubuf.buf = buf;
    d->u.ubuf.offset = offset;
    d->u.ubuf.maybeSize = size; // 0 = all
    d->u.ubuf.hasDynamicOffset = hasDynamicOffset;
}

void QSSGRhiShaderResourceBindingList::addTexture(int binding, QRhiShaderResourceBinding::StageFlags stage, QRhiTexture *tex, QRhiSampler *sampler)
{
#ifdef QT_DEBUG
    if (p == QSSGRhiShaderResourceBindingList::MAX_SIZE) {
        qWarning("Out of shader resource bindings slots (max is %d)", MAX_SIZE);
        return;
    }
#endif
    QRhiShaderResourceBinding::Data *d = QRhiImplementation::shaderResourceBindingData(v[p++]);
    h ^= qintptr(tex) ^ qintptr(sampler);
    d->binding = binding;
    d->stage = stage;
    d->type = QRhiShaderResourceBinding::SampledTexture;
    d->u.stex.count = 1;
    d->u.stex.texSamplers[0].tex = tex;
    d->u.stex.texSamplers[0].sampler = sampler;
}

void QSSGRhiShaderResourceBindingList::addImageLoad(int binding, QRhiShaderResourceBinding::StageFlags stage, QRhiTexture *tex, int level)
{
#ifdef QT_DEBUG
    if (p == QSSGRhiShaderResourceBindingList::MAX_SIZE) {
        qWarning("Out of shader resource bindings slots (max is %d)", MAX_SIZE);
        return;
    }
#endif
    QRhiShaderResourceBinding::Data *d = QRhiImplementation::shaderResourceBindingData(v[p++]);
    h ^= qintptr(tex) ^ qintptr(level);
    d->binding = binding;
    d->stage = stage;
    d->type = QRhiShaderResourceBinding::ImageLoad;
    d->u.simage.tex = tex;
    d->u.simage.level = level;
}

void QSSGRhiShaderResourceBindingList::addImageStore(int binding, QRhiShaderResourceBinding::StageFlags stage, QRhiTexture *tex, int level)
{
#ifdef QT_DEBUG
    if (p == QSSGRhiShaderResourceBindingList::MAX_SIZE) {
        qWarning("Out of shader resource bindings slots (max is %d)", MAX_SIZE);
        return;
    }
#endif
    QRhiShaderResourceBinding::Data *d = QRhiImplementation::shaderResourceBindingData(v[p++]);
    h ^= qintptr(tex) ^ qintptr(level);
    d->binding = binding;
    d->stage = stage;
    d->type = QRhiShaderResourceBinding::ImageStore;
    d->u.simage.tex = tex;
    d->u.simage.level = level;
}

void QSSGRhiShaderResourceBindingList::addImageLoadStore(int binding, QRhiShaderResourceBinding::StageFlags stage, QRhiTexture *tex, int level)
{
#ifdef QT_DEBUG
    if (p == QSSGRhiShaderResourceBindingList::MAX_SIZE) {
        qWarning("Out of shader resource bindings slots (max is %d)", MAX_SIZE);
        return;
    }
#endif
    QRhiShaderResourceBinding::Data *d = QRhiImplementation::shaderResourceBindingData(v[p++]);
    h ^= qintptr(tex) ^ qintptr(level);
    d->binding = binding;
    d->stage = stage;
    d->type = QRhiShaderResourceBinding::ImageLoadStore;
    d->u.simage.tex = tex;
    d->u.simage.level = level;
}

bool QSSGRhiContextPrivate::shaderDebuggingEnabled()
{
    static const bool isSet = (qEnvironmentVariableIntValue("QT_RHI_SHADER_DEBUG") != 0);
    return isSet;
}

bool QSSGRhiContextPrivate::editorMode()
{
    static const bool isSet = (qEnvironmentVariableIntValue("QT_QUICK3D_EDITORMODE") != 0);
    return isSet;
}

QRhiGraphicsPipeline *QSSGRhiContextPrivate::pipeline(const QSSGGraphicsPipelineStateKey &key,
                                                      QRhiRenderPassDescriptor *rpDesc,
                                                      QRhiShaderResourceBindings *srb)
{
    auto it = m_pipelines.constFind(key);
    if (it != m_pipelines.constEnd())
        return it.value();

           // Build a new one. This is potentially expensive.
    QRhiGraphicsPipeline *ps = m_rhi->newGraphicsPipeline();
    const auto &ia = QSSGRhiInputAssemblerStatePrivate::get(key.state);

    const auto *shaderPipeline = QSSGRhiGraphicsPipelineStatePrivate::getShaderPipeline(key.state);
    ps->setShaderStages(shaderPipeline->cbeginStages(), shaderPipeline->cendStages());
    ps->setVertexInputLayout(ia.inputLayout);
    ps->setShaderResourceBindings(srb);
    ps->setRenderPassDescriptor(rpDesc);

    QRhiGraphicsPipeline::Flags flags;
    if (key.state.flags.testFlag(QSSGRhiGraphicsPipelineState::Flag::UsesScissor))
        flags |= QRhiGraphicsPipeline::UsesScissor;

    static const bool shaderDebugInfo = qEnvironmentVariableIntValue("QT_QUICK3D_SHADER_DEBUG_INFO");
    if (shaderDebugInfo)
        flags |= QRhiGraphicsPipeline::CompileShadersWithDebugInfo;
    ps->setFlags(flags);

    ps->setTopology(ia.topology);
    ps->setCullMode(key.state.cullMode);
    if (ia.topology == QRhiGraphicsPipeline::Lines || ia.topology == QRhiGraphicsPipeline::LineStrip)
        ps->setLineWidth(key.state.lineWidth);

    const bool blendEnabled = key.state.flags.testFlag(QSSGRhiGraphicsPipelineState::Flag::BlendEnabled);
    QVarLengthArray<QRhiGraphicsPipeline::TargetBlend, 8> targetBlends(key.state.colorAttachmentCount);
    for (int i = 0; i < key.state.colorAttachmentCount; ++i) {
        targetBlends[i] =  key.state.targetBlend[i];
        targetBlends[i].enable = blendEnabled;
    }
    ps->setTargetBlends(targetBlends.cbegin(), targetBlends.cend());

    ps->setSampleCount(key.state.samples);
    ps->setMultiViewCount(key.state.viewCount);

    ps->setDepthTest(key.state.flags.testFlag(QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled));
    ps->setDepthWrite(key.state.flags.testFlag(QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled));
    ps->setDepthOp(key.state.depthFunc);

    ps->setDepthBias(key.state.depthBias);
    ps->setSlopeScaledDepthBias(key.state.slopeScaledDepthBias);
    ps->setPolygonMode(key.state.polygonMode);

    const bool usesStencilRef = (key.state.flags.testFlag(QSSGRhiGraphicsPipelineState::Flag::UsesStencilRef));
    if (usesStencilRef)
        flags |= QRhiGraphicsPipeline::UsesStencilRef;
    ps->setFlags(flags);
    ps->setStencilFront(key.state.stencilOpFrontState);
    ps->setStencilTest(usesStencilRef);
    ps->setStencilWriteMask(key.state.stencilWriteMask);

    if (!ps->create()) {
        qWarning("Failed to build graphics pipeline state");
        delete ps;
        return nullptr;
    }

    m_pipelines.insert(key, ps);
    return ps;
}

QRhiComputePipeline *QSSGRhiContextPrivate::computePipeline(const QSSGComputePipelineStateKey &key, QRhiShaderResourceBindings *srb)
{
    auto it = m_computePipelines.constFind(key);
    if (it != m_computePipelines.constEnd())
        return it.value();

    QRhiComputePipeline *computePipeline = m_rhi->newComputePipeline();
    computePipeline->setShaderResourceBindings(srb);
    computePipeline->setShaderStage({ QRhiShaderStage::Compute, key.shader });
    if (!computePipeline->create()) {
        qWarning("Failed to build compute pipeline");
        delete computePipeline;
        return nullptr;
    }
    m_computePipelines.insert(key, computePipeline);
    return computePipeline;
}

/*!
    \return The recommended flags when calling QRhiCommandBuffer::beginPass().
 */
QRhiCommandBuffer::BeginPassFlags QSSGRhiContext::commonPassFlags() const
{
    // We do not use GPU compute at all at the moment, this means we can
    // get a small performance gain with OpenGL by declaring this.
    return QRhiCommandBuffer::DoNotTrackResourcesForCompute;
}

QSSGRhiRenderableTextureV2::~QSSGRhiRenderableTextureV2()
{
    for (auto &tex : textures)
        tex->invalidate();
}

/*!
    \internal

    \fn void QSSGRhiRenderableTextureV2::setDescription(QRhi *rhi, QRhiTextureRenderTargetDescription rtDesc)

    Takes ownership of the textures in \a rtDesc and creates a render target.
    Once set the textures should be assume owned by this object references to
    individual textures can be gotten via the textures() method.
 */
void QSSGRhiRenderableTextureV2::setDescription(QRhi *rhi, QRhiTextureRenderTargetDescription rtDesc, QRhiTextureRenderTarget::Flags flags)
{
    Q_ASSERT(rhi);

    textures.clear();
    const size_t colorAttachmentCount = rtDesc.colorAttachmentCount();
    for (size_t i = 0; i < colorAttachmentCount; ++i)
        textures.push_back(std::make_unique<QSSGManagedRhiTexture>(m_manager, rtDesc.colorAttachmentAt(i)->texture(), QSSGManagedRhiTexture::Private::Initialize));

    if (rtDesc.depthTexture()) {
        depthTexture = std::make_unique<QSSGManagedRhiTexture>(m_manager, rtDesc.depthTexture(), QSSGManagedRhiTexture::Private::Initialize);
        depthStencil.reset();
    } else if (rtDesc.depthStencilBuffer()) {
        depthStencil.reset(rtDesc.depthStencilBuffer());
        depthTexture.reset();
    } else {
        depthTexture.reset();
        depthStencil = nullptr;
    }

    dirty = ColorTextureDirty | DepthTextureDirty | DepthStencilDirty;

    rt.reset(rhi->newTextureRenderTarget(rtDesc));
    rt->setName(rtName);
    rt->setFlags(flags);
    rpDesc.reset(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rpDesc.get());
    if (!rt->create()) {
        qWarning("Failed to create renderable texture render target");
        rt.reset();
        return;
    }
}

void QSSGRhiRenderableTextureV2::resetRenderTarget()
{
    rt.reset();
    rpDesc.reset();
}

void QSSGRhiRenderableTextureV2::reset()
{
    if (rt)
        rt->setDescription({/* empty */});

    resetRenderTarget();

    textures.clear();

    depthStencil = nullptr;
    depthTexture.reset();
}

// Called by the manager when it's being destroyed and since it owns the resource
// it will release them.
void QSSGRhiRenderableTextureV2::invalidate()
{
    // NOTE: This does not delete the texture wrappers, just releases the underlying QRhiTexture.
    for (auto &tex : textures)
        tex->invalidate();

    if (depthTexture)
        depthTexture->invalidate();

    reset();
}

QSSGManagedRhiTexture::QSSGManagedRhiTexture(const std::shared_ptr<QSSGUserRenderPassManager> &manager, QRhiTexture *texture, Private)
    : m_manager(manager)
    , m_texture(texture)
{
    Q_ASSERT(m_manager != nullptr);
    m_manager->registerManagedTexture(this);
}

QSSGManagedRhiTexture::QSSGManagedRhiTexture(const std::shared_ptr<QSSGUserRenderPassManager> &manager, std::unique_ptr<QRhiTexture> texture)
    : m_manager(manager)
    , m_texture(std::move(texture))
{
    Q_ASSERT(m_manager != nullptr);
    m_manager->registerManagedTexture(this);
}

QSSGManagedRhiTexture::~QSSGManagedRhiTexture()
{
    invalidate();
}

void QSSGManagedRhiTexture::invalidate()
{
    if (m_manager)
        m_manager->unregisterManagedTexture(this);

    if (Q_UNLIKELY(!m_manager && m_texture)) {
        qWarning("QSSGManagedRhiTexture: No manager for tracked resource!");
        delete m_texture.release();
    }

    // Under normal circumstances, the manager is responsible for deleting the texture.
    (void)m_texture.release();
}

QT_END_NAMESPACE

// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "graphobjects/qssgrendermodel_p.h"
#include "qssgdebugdrawsystem_p.h"

#include "qssgrendermesh_p.h"
#include "resourcemanager/qssgrenderbuffermanager_p.h"
#include "rendererimpl/qssgrenderableobjects_p.h"
#include "rendererimpl/qssglayerrenderdata_p.h"

QT_BEGIN_NAMESPACE

QSSGDebugDrawSystem::QSSGDebugDrawSystem()
{

}

QSSGDebugDrawSystem::~QSSGDebugDrawSystem()
{

}

bool QSSGDebugDrawSystem::hasContent() const
{
    return !m_lines.isEmpty() || !m_persistentLines.isEmpty() || !m_bounds.isEmpty() || !m_persistentBounds.isEmpty() || !m_persistentPoints.isEmpty() || !m_points.isEmpty();
}

void QSSGDebugDrawSystem::drawLine(const QVector3D &startPoint,
                                   const QVector3D &endPoint,
                                   const QColor &color,
                                   bool isPersistent)
{
    LineData line = {startPoint, endPoint, color};
    if (isPersistent)
        m_persistentLines.append(line);
    else
        m_lines.append(line);
}

void QSSGDebugDrawSystem::drawBounds(const QSSGBounds3 &bounds,
                                     const QColor &color,
                                     bool isPersistent)
{
    BoundsData bound = {bounds, color};
    if (isPersistent)
        m_persistentBounds.append(bound);
    else
        m_bounds.append(bound);
}

void QSSGDebugDrawSystem::drawPoint(const QVector3D &vertex, const QColor &color, bool isPersistent)
{
    VertexData point = {vertex, {color.redF(), color.greenF(), color.blueF()}};
    if (isPersistent)
        m_persistentPoints.append(point);
    else
        m_points.append(point);
}


void QSSGDebugDrawSystem::prepareGeometry(QSSGRhiContext *rhiCtx, QRhiResourceUpdateBatch *rub)
{

    QVector<VertexData> vertexData;
    QVector<quint32> indexData;
    QVector<VertexData> pointsData;
    for (const auto &line : m_persistentLines)
        generateLine(line, vertexData, indexData);
    for (const auto &line : m_lines)
        generateLine(line, vertexData, indexData);
    for (const auto &bounds : m_persistentBounds)
        generateBox(bounds, vertexData, indexData);
    for (const auto &bounds : m_bounds)
        generateBox(bounds, vertexData, indexData);
    pointsData = m_persistentPoints + m_points;

    if (!vertexData.isEmpty()) {
        // Lines
        QByteArray vertexBufferData(reinterpret_cast<const char*>(vertexData.constData()), qsizetype(vertexData.count() * 6 * sizeof(float)));
        QByteArray indexBufferData(reinterpret_cast<const char*>(indexData.constData()), qsizetype(indexData.count() * sizeof(quint32)));

        if (m_lineVertexBuffer)
            m_lineVertexBuffer.reset();
        if (m_lineIndexBuffer)
            m_lineIndexBuffer.reset();

        m_lineVertexBuffer = std::make_shared<QSSGRhiBuffer>(*rhiCtx,
                                                             QRhiBuffer::Immutable,
                                                             QRhiBuffer::VertexBuffer,
                                                             quint32(6 * sizeof(float)),
                                                             6 * sizeof(float) * vertexData.count());
        m_lineVertexBuffer->buffer()->setName(QByteArrayLiteral("debug lines vertex buffer"));
        rub->uploadStaticBuffer(m_lineVertexBuffer->buffer(), vertexBufferData.constData());

        m_lineIndexBuffer = std::make_shared<QSSGRhiBuffer>(*rhiCtx,
                                                            QRhiBuffer::Immutable,
                                                            QRhiBuffer::IndexBuffer,
                                                            0,
                                                            indexBufferData.size(),
                                                            QRhiCommandBuffer::IndexUInt32);
        m_lineIndexBuffer->buffer()->setName(QByteArrayLiteral("debug lines index buffer"));
        rub->uploadStaticBuffer(m_lineIndexBuffer->buffer(), indexBufferData.constData());

        m_indexSize = indexData.count();
    }

    if (!pointsData.isEmpty()) {
        // Points
        QByteArray vertexBufferData(reinterpret_cast<const char*>(pointsData.constData()), qsizetype(pointsData.count() * 6 * sizeof(float)));

        if (m_pointVertexBuffer)
            m_pointVertexBuffer.reset();

        m_pointVertexBuffer = std::make_shared<QSSGRhiBuffer>(*rhiCtx,
                                                              QRhiBuffer::Immutable,
                                                              QRhiBuffer::VertexBuffer,
                                                              quint32(6 * sizeof(float)),
                                                              vertexBufferData.size());
        m_pointVertexBuffer->buffer()->setName(QByteArrayLiteral("debug points vertex buffer"));
        rub->uploadStaticBuffer(m_pointVertexBuffer->buffer(), vertexBufferData.constData());
        m_pointsSize = pointsData.count();
    }
}

void QSSGDebugDrawSystem::recordRenderDebugObjects(QSSGRhiContext *rhiCtx,
                                                   QSSGRhiGraphicsPipelineState *ps,
                                                   QRhiShaderResourceBindings *srb,
                                                   QRhiRenderPassDescriptor *rpDesc)
{
    auto &ia = QSSGRhiInputAssemblerStatePrivate::get(*ps);
    ia.inputLayout.setAttributes({
                                         { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
                                         { 0, 1, QRhiVertexInputAttribute::Float3, 3 * sizeof(float) }
                                     });
    ia.inputs << QSSGRhiInputAssemblerState::PositionSemantic
                  << QSSGRhiInputAssemblerState::ColorSemantic;
    ia.inputLayout.setBindings({6 * sizeof(float)});
    ia.topology = QRhiGraphicsPipeline::Lines;
    ps->flags |= QSSGRhiGraphicsPipelineState::Flag::DepthWriteEnabled;
    ps->flags |= QSSGRhiGraphicsPipelineState::Flag::DepthTestEnabled;
    ps->cullMode = QRhiGraphicsPipeline::None;

    QSSGRhiContextPrivate *rhiCtxD = QSSGRhiContextPrivate::get(rhiCtx);
    QRhiCommandBuffer *cb = rhiCtx->commandBuffer();
    if (m_indexSize > 0) {
        auto graphicsPipeline = rhiCtxD->pipeline(*ps, rpDesc, srb);
        cb->setGraphicsPipeline(graphicsPipeline);
        cb->setShaderResources(srb);
        cb->setViewport(ps->viewport);

        // Lines
        QRhiCommandBuffer::VertexInput vb(m_lineVertexBuffer->buffer(), 0);
        cb->setVertexInput(0, 1, &vb, m_lineIndexBuffer->buffer(), 0, m_lineIndexBuffer->indexFormat());
        cb->drawIndexed(m_indexSize);
    }

    // Points
    if (m_pointsSize > 0) {
        ia.topology = QRhiGraphicsPipeline::Points;
        auto graphicsPipeline = rhiCtxD->pipeline(*ps, rpDesc, srb);
        cb->setGraphicsPipeline(graphicsPipeline);
        cb->setShaderResources(srb);
        cb->setViewport(ps->viewport);

        QRhiCommandBuffer::VertexInput vb(m_pointVertexBuffer->buffer(), 0);
        cb->setVertexInput(0, 1, &vb);
        cb->draw(m_pointsSize);
    }

    m_lines.clear();
    m_bounds.clear();
    m_points.clear();
    m_indexSize = 0;
    m_pointsSize = 0;
}

void QSSGDebugDrawSystem::setEnabled(bool v)
{
    modes = v ? (modes | ModeFlagT(Mode::Other)) : (modes & ~ModeFlagT(Mode::Other));
}

void QSSGDebugDrawSystem::generateLine(const LineData &line, QVector<VertexData> &vertexArray, QVector<quint32> &indexArray)
{
    const QVector3D color = {line.color.redF(), line.color.greenF(), line.color.blueF()};
    indexArray.append(vertexArray.count());
    vertexArray.append({line.startPoint, color});
    indexArray.append(vertexArray.count());
    vertexArray.append({line.endPoint, color});
}

void QSSGDebugDrawSystem::generateBox(const BoundsData &bounds, QVector<VertexData> &vertexArray, QVector<quint32> &indexArray)
{
    const QVector3D color = {bounds.color.redF(), bounds.color.greenF(), bounds.color.blueF()};

    quint32 offset = vertexArray.count();
    for (const QVector3D point : bounds.bounds.toQSSGBoxPoints())
        vertexArray.append({point, color});

    indexArray.append(offset + 0);
    indexArray.append(offset + 3);

    indexArray.append(offset + 3);
    indexArray.append(offset + 6);

    indexArray.append(offset + 6);
    indexArray.append(offset + 1);

    indexArray.append(offset + 1);
    indexArray.append(offset + 0);

    indexArray.append(offset + 2);
    indexArray.append(offset + 5);

    indexArray.append(offset + 5);
    indexArray.append(offset + 4);

    indexArray.append(offset + 4);
    indexArray.append(offset + 7);

    indexArray.append(offset + 7);
    indexArray.append(offset + 2);

    indexArray.append(offset + 0);
    indexArray.append(offset + 2);

    indexArray.append(offset + 3);
    indexArray.append(offset + 5);

    indexArray.append(offset + 6);
    indexArray.append(offset + 4);

    indexArray.append(offset + 1);
    indexArray.append(offset + 7);

}

QColor QSSGDebugDrawSystem::levelOfDetailColor(quint32 lod)
{
    static const QColor colors[] {
        QColor(Qt::white),
        QColor(Qt::red),
        QColor(Qt::green),
        QColor(Qt::blue),
        QColor(Qt::yellow),
        QColor(Qt::cyan),
        QColor(Qt::magenta),
        QColor(Qt::darkRed),
        QColor(Qt::darkGreen),
        QColor(Qt::darkBlue),
        QColor(Qt::darkCyan),
        QColor(Qt::darkMagenta),
        QColor(Qt::darkYellow),
        QColor(Qt::darkGray)
    };

    const size_t idx = qBound<size_t>(0, lod, std::size(colors) - 1);
    return colors[idx];
}

void QSSGDebugDrawSystem::debugNormals(QSSGBufferManager &bufferManager, const QSSGModelContext &theModelContext, const QSSGRenderSubset &theSubset, quint32 subsetLevelOfDetail, float lineLength)
{
    const auto &model = theModelContext.model;

    QSSGMesh::Mesh mesh;
    if (model.geometry)
        mesh = bufferManager.loadMeshData(model.geometry);
    else
        mesh = bufferManager.loadMeshData(model.meshPath);

    if (!mesh.isValid())
        return; // invalid mesh

    QByteArray vertexData = mesh.vertexBuffer().data;
    if (vertexData.isEmpty())
        return; // no vertex dat
    quint32 vertexStride = mesh.vertexBuffer().stride;
    QByteArray indexData = mesh.indexBuffer().data;
    if (indexData.isEmpty())
        return; // no index data, not what we're after
    if (mesh.indexBuffer().componentType != QSSGMesh::Mesh::ComponentType::UnsignedInt32)
        return; // not uint3d, not what we're after either

    quint32 positionOffset = UINT_MAX;
    quint32 normalOffset = UINT_MAX;

    for (const QSSGMesh::Mesh::VertexBufferEntry &vbe : mesh.vertexBuffer().entries) {
        if (vbe.name == QSSGMesh::MeshInternal::getPositionAttrName()) {
            positionOffset = vbe.offset;
            if (vbe.componentType != QSSGMesh::Mesh::ComponentType::Float32 &&
                vbe.componentCount != 3)
                return; // not a vec3, some weird stuff
        } else if (vbe.name == QSSGMesh::MeshInternal::getNormalAttrName()) {
            normalOffset = vbe.offset;
            if (vbe.componentType != QSSGMesh::Mesh::ComponentType::Float32 &&
                vbe.componentCount != 3)
                return; // not a vec3, really weird normals I guess
        }
    }

    const auto &globalTransform = theModelContext.globalTransform;
    // Draw original vertex normals as blue lines
    {
        // Get Indexes
        const quint32 *p = reinterpret_cast<const quint32 *>(indexData.constData());
        const char *vp = vertexData.constData();
        p += theSubset.offset;
        for (uint i = 0; i < theSubset.count; ++i) {
            const quint32 index = *(p + i);
            const char * posPtr = vp + (index * vertexStride) + positionOffset;
            const float *fPosPtr = reinterpret_cast<const float *>(posPtr);
            QVector3D position(fPosPtr[0], fPosPtr[1], fPosPtr[2]);
            const char * normalPtr = vp + (index * vertexStride) + normalOffset;
            const float *fNormalPtr = reinterpret_cast<const float *>(normalPtr);
            QVector3D normal(fNormalPtr[0], fNormalPtr[1], fNormalPtr[2]);
            position = globalTransform.map(position);
            normal = QSSGUtils::mat33::transform(theModelContext.normalMatrix, normal);
            normal = normal.normalized();
            drawLine(position, position + (normal * lineLength), QColor(Qt::blue));
        }
    }

           // Draw lod vertex normals as red lines
    if (subsetLevelOfDetail != 0) {
        // Get Indexes
        const quint32 *p = reinterpret_cast<const quint32 *>(indexData.constData());
        const char *vp = vertexData.constData();
        p += theSubset.lodOffset(subsetLevelOfDetail);
        const quint32 indexCount = theSubset.lodCount(subsetLevelOfDetail);
        for (uint i = 0; i < indexCount; ++i) {
            const quint32 index = *(p + i);
            const char * posPtr = vp + (index * vertexStride) + positionOffset;
            const float *fPosPtr = reinterpret_cast<const float *>(posPtr);
            QVector3D position(fPosPtr[0], fPosPtr[1], fPosPtr[2]);
            const char * normalPtr = vp + (index * vertexStride) + normalOffset;
            const float *fNormalPtr = reinterpret_cast<const float *>(normalPtr);
            QVector3D normal(fNormalPtr[0], fNormalPtr[1], fNormalPtr[2]);
            position = globalTransform.map(position);
            normal = QSSGUtils::mat33::transform(theModelContext.normalMatrix, normal);
            normal = normal.normalized();
            drawLine(position, position + (normal * lineLength), QColor(Qt::red));
        }
    }
}

QT_END_NAMESPACE

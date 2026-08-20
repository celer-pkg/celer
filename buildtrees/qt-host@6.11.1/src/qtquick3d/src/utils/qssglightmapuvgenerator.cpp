// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssglightmapuvgenerator_p.h"
#include "xatlas.h"

QT_BEGIN_NAMESPACE

QSSGLightmapUVGeneratorResult QSSGLightmapUVGenerator::run(const QByteArray &positions,
                                                           const QByteArray &normals,
                                                           const QByteArray &uv0,
                                                           const QByteArray &index,
                                                           QSSGMesh::Mesh::ComponentType indexComponentType,
                                                           float texelsPerUnit)
{
    QSSGLightmapUVGeneratorResult result;

    xatlas::MeshDecl meshInfo;

    if (indexComponentType == QSSGMesh::Mesh::ComponentType::UnsignedInt16) {
        meshInfo.indexFormat = xatlas::IndexFormat::UInt16;
    } else if (indexComponentType == QSSGMesh::Mesh::ComponentType::UnsignedInt32) {
        meshInfo.indexFormat = xatlas::IndexFormat::UInt32;
    } else {
        qWarning("Lightmap UV generator: Unknown index type %d; cannot generate",
                 int(indexComponentType));
        return result;
    }

    const quint32 indexComponentByteSize = QSSGMesh::MeshInternal::byteSizeForComponentType(indexComponentType);
    const quint32 indexCount = index.size() / indexComponentByteSize;

    meshInfo.indexCount = indexCount;
    meshInfo.indexData = index.constData();

    const quint32 positionStride = 3 * sizeof(float);
    const quint32 normalStride = 3 * sizeof(float);
    const quint32 uvStride = 2 * sizeof(float);

    meshInfo.vertexCount = positions.size() / positionStride;
    meshInfo.vertexPositionData = positions.data();
    meshInfo.vertexPositionStride = positionStride;

    if (!normals.isEmpty()) {
        meshInfo.vertexNormalData = normals.constData();
        meshInfo.vertexNormalStride = normalStride;
    } else {
        meshInfo.vertexNormalData = nullptr;
        meshInfo.vertexNormalStride = 0;
    }

    if (!uv0.isEmpty()) {
        meshInfo.vertexUvData = uv0.constData();
        meshInfo.vertexUvStride = uvStride;
    } else {
        meshInfo.vertexUvData = nullptr;
        meshInfo.vertexUvStride = 0;
    }

    xatlas::PackOptions packOptions;
    packOptions.maxChartSize = 4096;
    packOptions.padding = 1;
    packOptions.resolution = 0;
    packOptions.texelsPerUnit = texelsPerUnit;
    packOptions.blockAlign = true;

    xatlas::ChartOptions chartOptions;

    xatlas::Atlas *atlas = xatlas::Create();
    xatlas::AddMeshError err = xatlas::AddMesh(atlas, meshInfo, 1);
    if (err != xatlas::AddMeshError::Success) {
        qWarning("Failed to register mesh for UV unwrapping (error %d)", int(err));
        xatlas::Destroy(atlas);
        return result;
    }
    xatlas::Generate(atlas, chartOptions, packOptions);

    const uint32_t textureWidth = atlas->width;
    const uint32_t textureHeight = atlas->height;
    if (textureWidth == 0 || textureHeight == 0) {
        qWarning("Texture size is empty, UV unwrapping failed");
        xatlas::Destroy(atlas);
        return result;
    }
    result.lightmapWidth = textureWidth;
    result.lightmapHeight = textureHeight;

    const xatlas::Mesh &output = atlas->meshes[0];
    result.lightmapUVChannel.resize(output.vertexCount * uvStride);
    result.vertexMap.resize(output.vertexCount);

    float *uvPtr = reinterpret_cast<float *>(result.lightmapUVChannel.data());
    for (uint32_t i = 0; i < output.vertexCount; ++i) {
        const float u = output.vertexArray[i].uv[0] / float(textureWidth);
        const float v = output.vertexArray[i].uv[1] / float(textureHeight);
        *uvPtr++ = u;
        *uvPtr++ = v;
        result.vertexMap[i] = output.vertexArray[i].xref;
    }

    result.indexData.resize(output.indexCount * sizeof(quint32));
    quint32 *indexPtr = reinterpret_cast<quint32 *>(result.indexData.data());
    for (uint32_t i = 0; i < output.indexCount; ++i)
        *indexPtr++ = output.indexArray[i];

    xatlas::Destroy(atlas);

    return result;
}

QT_END_NAMESPACE

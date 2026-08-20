// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgmesh_p.h"

#include <QtCore/QVector>
#include <QtQuick3DUtils/private/qssgdataref_p.h>
#include <QtQuick3DUtils/private/qssglightmapuvgenerator_p.h>

#include "meshoptimizer.h"

#include <algorithm>

QT_BEGIN_NAMESPACE

namespace QSSGMesh {

// fileId, fileVersion, offset, count
static const size_t MULTI_HEADER_STRUCT_SIZE = 16;

// meshOffset, meshId, padding
static const size_t MULTI_ENTRY_STRUCT_SIZE = 16;

// fileId, fileVersion, flags, size
static const size_t MESH_HEADER_STRUCT_SIZE = 12;

// vertexBuffer, indexBuffer, subsets, joints, drawMode, winding
static const size_t MESH_STRUCT_SIZE = 56;

// vertex buffer entry list: nameOffset, componentType, componentCount, offset
static const size_t VERTEX_BUFFER_ENTRY_STRUCT_SIZE = 16;

// subset list: count, offset, minXYZ, maxXYZ, nameOffset, nameLength
static const size_t SUBSET_STRUCT_SIZE_V3_V4 = 40;
// subset list: count, offset, minXYZ, maxXYZ, nameOffset, nameLength, lightmapSizeWidth, lightmapSizeHeight
static const size_t SUBSET_STRUCT_SIZE_V5 = 48;
// subset list: count, offset, minXYZ, maxXYZ, nameOffset, nameLength, lightmapSizeWidth, lightmapSizeHeight, lodCount
static const size_t SUBSET_STRUCT_SIZE_V6 = 52;

//lod entry: count, offset, distance
static const size_t LOD_STRUCT_SIZE = 12;

MeshInternal::MultiMeshInfo MeshInternal::readFileHeader(QIODevice *device)
{
    const qint64 multiHeaderStartOffset = device->size() - qint64(MULTI_HEADER_STRUCT_SIZE);

    device->seek(multiHeaderStartOffset);
    QDataStream inputStream(device);
    inputStream.setByteOrder(QDataStream::LittleEndian);
    inputStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    MultiMeshInfo meshFileInfo;
    inputStream >> meshFileInfo.fileId >> meshFileInfo.fileVersion;

    if (!meshFileInfo.isValid()) {
        qWarning("Mesh file invalid");
        return {};
    }

    quint32 multiEntriesOffset; // unused, the entry list is right before the header
    quint32 meshCount;
    inputStream >> multiEntriesOffset >> meshCount;

    for (quint32 i = 0; i < meshCount; ++i) {
        device->seek(multiHeaderStartOffset
                     - (qint64(MULTI_ENTRY_STRUCT_SIZE) * meshCount)
                     + (qint64(MULTI_ENTRY_STRUCT_SIZE) * i));
        quint64 offset;
        quint32 id;
        inputStream >> offset >> id;
        meshFileInfo.meshEntries.insert(id, offset);
    }

    return meshFileInfo;
}

void MeshInternal::writeFileHeader(QIODevice *device, const MeshInternal::MultiMeshInfo &meshFileInfo)
{
    QDataStream outputStream(device);
    outputStream.setByteOrder(QDataStream::LittleEndian);
    outputStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    const quint32 multiEntriesOffset = device->pos();
    for (auto it = meshFileInfo.meshEntries.cbegin(), end = meshFileInfo.meshEntries.cend(); it != end; ++it) {
        const quint32 id = it.key();
        const quint64 offset = it.value();
        const quint32 padding = 0;
        outputStream << offset << id << padding;
    }

    const quint32 meshCount = meshFileInfo.meshEntries.size();
    outputStream << meshFileInfo.fileId << meshFileInfo.fileVersion << multiEntriesOffset << meshCount;
}

quint64 MeshInternal::readMeshData(QIODevice *device, quint64 offset, Mesh *mesh, MeshDataHeader *header)
{
    static char alignPadding[4] = {};

    device->seek(offset);
    QDataStream inputStream(device);
    inputStream.setByteOrder(QDataStream::LittleEndian);
    inputStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    inputStream >> header->fileId >> header->fileVersion >> header->flags >> header->sizeInBytes;
    if (!header->isValid()) {
        qWarning() << "Mesh data invalid";
        if (header->fileId == MeshDataHeader::FILE_ID) {
            if (header->fileVersion > MeshDataHeader::FILE_VERSION)
                qWarning() << "File version " << header->fileVersion << " newer than " << MeshDataHeader::FILE_VERSION;
            if (header->fileVersion < MeshDataHeader::LEGACY_MESH_FILE_VERSION)
                qWarning() << "File version " << header->fileVersion << " older than " << MeshDataHeader::LEGACY_MESH_FILE_VERSION;
        } else {
            qWarning() << "Invalid file ID" << header->fileId;
        }
        return 0;
    }

    MeshInternal::MeshOffsetTracker offsetTracker(offset + MESH_HEADER_STRUCT_SIZE);
    Q_ASSERT(offsetTracker.offset() == device->pos());

    quint32 targetBufferEntriesCount;
    quint32 vertexBufferEntriesCount;
    quint32 targetBufferDataSize;
    quint32 vertexBufferDataSize;
    inputStream >> targetBufferEntriesCount
                >> vertexBufferEntriesCount
                >> mesh->m_vertexBuffer.stride
                >> targetBufferDataSize
                >> vertexBufferDataSize;

    if (!header->hasSeparateTargetBuffer()) {
        targetBufferEntriesCount = 0;
        targetBufferDataSize = 0;
    }

    quint32 indexBufferComponentType;
    quint32 indexBufferDataOffset;
    quint32 indexBufferDataSize;
    inputStream >> indexBufferComponentType
                >> indexBufferDataOffset
                >> indexBufferDataSize;
    mesh->m_indexBuffer.componentType = Mesh::ComponentType(indexBufferComponentType);

    quint32 targetCount;
    quint32 subsetsCount;
    inputStream >> targetCount >> subsetsCount;
    mesh->m_targetBuffer.numTargets = targetCount;

    quint32 jointsOffsets; // unused
    quint32 jointsCount; // unused
    inputStream >> jointsOffsets >> jointsCount;
    quint32 drawMode;
    quint32 winding;
    inputStream >> drawMode >> winding;
    mesh->m_drawMode = Mesh::DrawMode(drawMode);
    mesh->m_winding = Mesh::Winding(winding);

    offsetTracker.advance(MESH_STRUCT_SIZE);

    quint32 entriesByteSize = 0;
    for (quint32 i = 0; i < vertexBufferEntriesCount; ++i) {
        Mesh::VertexBufferEntry vertexBufferEntry;
        quint32 componentType;
        quint32 nameOffset; // unused
        inputStream >> nameOffset
                    >> componentType
                    >> vertexBufferEntry.componentCount
                    >> vertexBufferEntry.offset;
        vertexBufferEntry.componentType = Mesh::ComponentType(componentType);
        mesh->m_vertexBuffer.entries.append(vertexBufferEntry);
        entriesByteSize += VERTEX_BUFFER_ENTRY_STRUCT_SIZE;
    }
    quint32 alignAmount = offsetTracker.alignedAdvance(entriesByteSize);
    if (alignAmount)
        device->read(alignPadding, alignAmount);

    // vertex buffer entry names
    quint32 numTargets = 0;
    // used for recording the target attributes supported by the mesh
    // and re-construting it when meeting attr_unsupported
    QList<QByteArray> attrNames;
    for (auto &entry : mesh->m_vertexBuffer.entries) {
        quint32 nameLength;
        inputStream >> nameLength;
        offsetTracker.advance(sizeof(quint32));
        const QByteArray nameWithZeroTerminator = device->read(nameLength);
        entry.name = QByteArray(nameWithZeroTerminator.constData(), qMax(0, nameWithZeroTerminator.size() - 1));
        alignAmount = offsetTracker.alignedAdvance(nameLength);
        if (alignAmount)
            device->read(alignPadding, alignAmount);
        // Old morph meshes' target attributes were appended sequentially
        // behind vertex attributes. However, since the number of targets are restricted by 8
        // the other attributes were named by "attr_unsupported"
        // So just checking numTargets is safe with the above assumption and
        // it will try to reconstruct the unsupported attributes.
        if (numTargets > 0 || (!header->hasSeparateTargetBuffer() && entry.name.startsWith("attr_t"))) {
            if (entry.name.sliced(6).startsWith("pos")) {
                const quint32 targetId = entry.name.mid(9).toUInt();
                // All the attributes of the first target should be recorded correctly.
                if (targetId == 0)
                    attrNames.append(MeshInternal::getPositionAttrName());
                numTargets = qMax(numTargets, targetId + 1);
                entry.name = MeshInternal::getPositionAttrName();
                mesh->m_targetBuffer.entries.append(entry);
                targetBufferEntriesCount++;
            } else if (entry.name.sliced(6).startsWith("norm")) {
                const quint32 targetId = entry.name.mid(10).toUInt();
                if (targetId == 0)
                    attrNames.append(MeshInternal::getNormalAttrName());
                numTargets = qMax(numTargets, targetId + 1);
                entry.name = MeshInternal::getNormalAttrName();
                mesh->m_targetBuffer.entries.append(entry);
                targetBufferEntriesCount++;
            } else if (entry.name.sliced(6).startsWith("tan")) {
                const quint32 targetId = entry.name.mid(9).toUInt();
                if (targetId == 0)
                    attrNames.append(MeshInternal::getTexTanAttrName());
                numTargets = qMax(numTargets, targetId + 1);
                entry.name = MeshInternal::getTexTanAttrName();
                mesh->m_targetBuffer.entries.append(entry);
                targetBufferEntriesCount++;
            } else if (entry.name.sliced(6).startsWith("binorm")) {
                const quint32 targetId = entry.name.mid(12).toUInt();
                if (targetId == 0)
                    attrNames.append(MeshInternal::getTexBinormalAttrName());
                numTargets = qMax(numTargets, targetId + 1);
                entry.name = MeshInternal::getTexBinormalAttrName();
                mesh->m_targetBuffer.entries.append(entry);
                targetBufferEntriesCount++;
            } else if (entry.name.startsWith("attr_unsupported")) {
                // Reconstruct
                entry.name = attrNames[targetBufferEntriesCount % attrNames.size()];
                mesh->m_targetBuffer.entries.append(entry);
                targetBufferEntriesCount++;
            }
        }
    }

    mesh->m_vertexBuffer.data = device->read(vertexBufferDataSize);
    alignAmount = offsetTracker.alignedAdvance(vertexBufferDataSize);
    if (alignAmount)
        device->read(alignPadding, alignAmount);

    mesh->m_indexBuffer.data = device->read(indexBufferDataSize);
    alignAmount = offsetTracker.alignedAdvance(indexBufferDataSize);
    if (alignAmount)
        device->read(alignPadding, alignAmount);

    quint32 subsetByteSize = 0;
    QVector<MeshInternal::Subset> internalSubsets;
    for (quint32 i = 0; i < subsetsCount; ++i) {
        MeshInternal::Subset subset;
        float minX;
        float minY;
        float minZ;
        float maxX;
        float maxY;
        float maxZ;
        quint32 nameOffset; // unused
        inputStream >> subset.count
                    >> subset.offset
                    >> minX
                    >> minY
                    >> minZ
                    >> maxX
                    >> maxY
                    >> maxZ
                    >> nameOffset
                    >> subset.nameLength;
        subset.bounds.min = QVector3D(minX, minY, minZ);
        subset.bounds.max = QVector3D(maxX, maxY, maxZ);
        if (header->hasLightmapSizeHint()) {
            quint32 width = 0;
            quint32 height = 0;
            inputStream >> width >> height;
            subset.lightmapSizeHint = QSize(width, height);
            if (header->hasLodDataHint()) {
                quint32 lodCount = 0;
                inputStream >> lodCount;
                subset.lodCount = lodCount;
                subsetByteSize += SUBSET_STRUCT_SIZE_V6;
            } else {
                subsetByteSize += SUBSET_STRUCT_SIZE_V5;
            }
        } else {
            subset.lightmapSizeHint = QSize(0, 0);
            subsetByteSize += SUBSET_STRUCT_SIZE_V3_V4;
        }
        internalSubsets.append(subset);

    }
    alignAmount = offsetTracker.alignedAdvance(subsetByteSize);
    if (alignAmount)
        device->read(alignPadding, alignAmount);

    for (MeshInternal::Subset &internalSubset : internalSubsets) {
        internalSubset.rawNameUtf16 = device->read(internalSubset.nameLength * 2); //UTF_16_le
        alignAmount = offsetTracker.alignedAdvance(internalSubset.nameLength * 2);
        if (alignAmount)
            device->read(alignPadding, alignAmount);
    }

    quint32 lodByteSize = 0;
    for (const MeshInternal::Subset &internalSubset : internalSubsets) {
        auto meshSubset = internalSubset.toMeshSubset();
        // Read Level of Detail data here
        for (auto &lod : meshSubset.lods) {
            quint32 count = 0;
            quint32 offset = 0;
            float distance = 0.0;
            inputStream >> count >> offset >> distance;
            lod.count = count;
            lod.offset = offset;
            lod.distance = distance;
            lodByteSize += LOD_STRUCT_SIZE;
        }

        mesh->m_subsets.append(meshSubset);
    }
    alignAmount = offsetTracker.alignedAdvance(lodByteSize);
    if (alignAmount)
        device->read(alignPadding, alignAmount);


    // Data for morphTargets
    if (targetBufferEntriesCount > 0) {
        if (header->hasSeparateTargetBuffer()) {
            entriesByteSize = 0;
            for (quint32 i = 0; i < targetBufferEntriesCount; ++i) {
                Mesh::VertexBufferEntry targetBufferEntry;
                quint32 componentType;
                quint32 nameOffset; // unused
                inputStream >> nameOffset
                            >> componentType
                            >> targetBufferEntry.componentCount
                            >> targetBufferEntry.offset;
                targetBufferEntry.componentType = Mesh::ComponentType(componentType);
                mesh->m_targetBuffer.entries.append(targetBufferEntry);
                entriesByteSize += VERTEX_BUFFER_ENTRY_STRUCT_SIZE;
            }
            alignAmount = offsetTracker.alignedAdvance(entriesByteSize);
            if (alignAmount)
                device->read(alignPadding, alignAmount);

            for (auto &entry : mesh->m_targetBuffer.entries) {
                quint32 nameLength;
                inputStream >> nameLength;
                offsetTracker.advance(sizeof(quint32));
                const QByteArray nameWithZeroTerminator = device->read(nameLength);
                entry.name = QByteArray(nameWithZeroTerminator.constData(), qMax(0, nameWithZeroTerminator.size() - 1));
                alignAmount = offsetTracker.alignedAdvance(nameLength);
                if (alignAmount)
                    device->read(alignPadding, alignAmount);
            }

            mesh->m_targetBuffer.data = device->read(targetBufferDataSize);
        } else {
            // remove target entries from vertexbuffer entries
            mesh->m_vertexBuffer.entries.remove(vertexBufferEntriesCount - targetBufferEntriesCount,
                                                targetBufferEntriesCount);
            const quint32 vertexCount = vertexBufferDataSize / mesh->m_vertexBuffer.stride;
            const quint32 targetEntryTexWidth = qCeil(qSqrt(vertexCount));
            const quint32 targetCompStride = targetEntryTexWidth * targetEntryTexWidth * 4 * sizeof(float);
            mesh->m_targetBuffer.data.resize(targetCompStride * targetBufferEntriesCount);
            const quint32 numComps = targetBufferEntriesCount / numTargets;
            for (quint32 i = 0; i < targetBufferEntriesCount; ++i) {
                auto &entry = mesh->m_targetBuffer.entries[i];
                char *dstBuf = mesh->m_targetBuffer.data.data()
                                + (i / numComps) * targetCompStride
                                + (i % numComps) * (targetCompStride * numTargets);
                const char *srcBuf = mesh->m_vertexBuffer.data.constData() + entry.offset;
                for (quint32 j = 0; j < vertexCount; ++j) {
                    // The number of old target components is fixed as 3
                    memcpy(dstBuf + j * 4 * sizeof(float),
                           srcBuf + j * mesh->m_vertexBuffer.stride,
                           3 * sizeof(float));
                }
                entry.offset = i * targetCompStride;
            }
            // now we don't need to have redundant targetbuffer entries
            mesh->m_targetBuffer.entries.remove(numComps, targetBufferEntriesCount - numComps);
            mesh->m_targetBuffer.numTargets = numTargets;
        }
    }

    return header->sizeInBytes;
}

void MeshInternal::writeMeshHeader(QIODevice *device, const MeshDataHeader &header)
{
    QDataStream outputStream(device);
    outputStream.setByteOrder(QDataStream::LittleEndian);
    outputStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    outputStream << header.fileId << header.fileVersion << header.flags << header.sizeInBytes;
}

// The legacy, now-removed, insane mesh code used to use a "serialization"
// strategy with dumping memory, yet combined with with an in-memory layout
// that is different from what's in the file. In version 4 we no longer write
// out valid offset values (see the // legacy offset comments), because the new
// loader does not need them, and calculating them is not sensible, especially
// due to the different layouts. We still do the alignment padding, even though
// that's also legacy nonsense, but having that allows the reader not have to
// branch based on the version.

quint64 MeshInternal::writeMeshData(QIODevice *device, const Mesh &mesh)
{
    static const char alignPadding[4] = {};

    QDataStream outputStream(device);
    outputStream.setByteOrder(QDataStream::LittleEndian);
    outputStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    const qint64 startPos = device->pos();
    MeshInternal::MeshOffsetTracker offsetTracker(startPos);
    Q_ASSERT(offsetTracker.offset() == device->pos());

    const quint32 vertexBufferEntriesCount = mesh.m_vertexBuffer.entries.size();
    const quint32 vertexBufferDataSize = mesh.m_vertexBuffer.data.size();
    const quint32 vertexBufferStride = mesh.m_vertexBuffer.stride;
    const quint32 targetBufferEntriesCount = mesh.m_targetBuffer.entries.count();
    const quint32 targetBufferDataSize = mesh.m_targetBuffer.data.size();
    outputStream << targetBufferEntriesCount
                 << vertexBufferEntriesCount
                 << vertexBufferStride;
    outputStream << targetBufferDataSize
                 << vertexBufferDataSize;

    const quint32 indexBufferDataSize = mesh.m_indexBuffer.data.size();
    const quint32 indexComponentType = quint32(mesh.m_indexBuffer.componentType);
    outputStream << indexComponentType;
    outputStream << quint32(0) // legacy offset
                 << indexBufferDataSize;

    const quint32 targetCount = mesh.m_targetBuffer.numTargets;
    const quint32 subsetsCount = mesh.m_subsets.size();
    outputStream << targetCount
                 << subsetsCount;

    outputStream << quint32(0) // legacy offset
                 << quint32(0); // legacy jointsCount

    const quint32 drawMode = quint32(mesh.m_drawMode);
    const quint32 winding = quint32(mesh.m_winding);
    outputStream << drawMode
                 << winding;

    offsetTracker.advance(MESH_STRUCT_SIZE);

    quint32 entriesByteSize = 0;
    for (quint32 i = 0; i < vertexBufferEntriesCount; ++i) {
        const Mesh::VertexBufferEntry &entry(mesh.m_vertexBuffer.entries[i]);
        const quint32 componentType = quint32(entry.componentType);
        const quint32 componentCount = entry.componentCount;
        const quint32 offset = entry.offset;
        outputStream << quint32(0) // legacy offset
                     << componentType
                     << componentCount
                     << offset;
        entriesByteSize += VERTEX_BUFFER_ENTRY_STRUCT_SIZE;
    }
    quint32 alignAmount = offsetTracker.alignedAdvance(entriesByteSize);
    if (alignAmount)
        device->write(alignPadding, alignAmount);

    for (quint32 i = 0; i < vertexBufferEntriesCount; ++i) {
        const Mesh::VertexBufferEntry &entry(mesh.m_vertexBuffer.entries[i]);
        const quint32 nameLength = entry.name.size() + 1;
        outputStream << nameLength;
        device->write(entry.name.constData(), nameLength); // with zero terminator included
        alignAmount = offsetTracker.alignedAdvance(sizeof(quint32) + nameLength);
        if (alignAmount)
            device->write(alignPadding, alignAmount);
    }

    device->write(mesh.m_vertexBuffer.data.constData(), vertexBufferDataSize);
    alignAmount = offsetTracker.alignedAdvance(vertexBufferDataSize);
    if (alignAmount)
        device->write(alignPadding, alignAmount);

    device->write(mesh.m_indexBuffer.data.constData(), indexBufferDataSize);
    alignAmount = offsetTracker.alignedAdvance(indexBufferDataSize);
    if (alignAmount)
        device->write(alignPadding, alignAmount);

    quint32 subsetByteSize = 0;
    for (quint32 i = 0; i < subsetsCount; ++i) {
        const Mesh::Subset &subset(mesh.m_subsets[i]);
        const quint32 subsetCount = subset.count;
        const quint32 subsetOffset = subset.offset;
        const float minX = subset.bounds.min.x();
        const float minY = subset.bounds.min.y();
        const float minZ = subset.bounds.min.z();
        const float maxX = subset.bounds.max.x();
        const float maxY = subset.bounds.max.y();
        const float maxZ = subset.bounds.max.z();
        const quint32 nameLength = subset.name.size() + 1;
        const quint32 lightmapSizeHintWidth = qMax(0, subset.lightmapSizeHint.width());
        const quint32 lightmapSizeHintHeight = qMax(0, subset.lightmapSizeHint.height());
        const quint32 lodCount = subset.lods.size();
        outputStream << subsetCount
                     << subsetOffset
                     << minX
                     << minY
                     << minZ
                     << maxX
                     << maxY
                     << maxZ;
        outputStream << quint32(0) // legacy offset
                     << nameLength;
        outputStream << lightmapSizeHintWidth
                     << lightmapSizeHintHeight;
        outputStream << lodCount;
        subsetByteSize += SUBSET_STRUCT_SIZE_V6;
    }
    alignAmount = offsetTracker.alignedAdvance(subsetByteSize);
    if (alignAmount)
        device->write(alignPadding, alignAmount);

    for (quint32 i = 0; i < subsetsCount; ++i) {
        const Mesh::Subset &subset(mesh.m_subsets[i]);
        const char *utf16Name = reinterpret_cast<const char *>(subset.name.utf16());
        const quint32 nameByteSize = (subset.name.size() + 1) * 2;
        device->write(utf16Name, nameByteSize);
        alignAmount = offsetTracker.alignedAdvance(nameByteSize);
        if (alignAmount)
            device->write(alignPadding, alignAmount);
    }

    // LOD data
    quint32 lodDataByteSize = 0;
    for (quint32 i = 0; i < subsetsCount; ++i) {
        const Mesh::Subset &subset(mesh.m_subsets[i]);
        for (auto lod : subset.lods) {
            const quint32 count = lod.count;
            const quint32 offset = lod.offset;
            const float distance = lod.distance;
            outputStream << count << offset << distance;
            lodDataByteSize += LOD_STRUCT_SIZE;
        }
    }
    alignAmount = offsetTracker.alignedAdvance(lodDataByteSize);
    if (alignAmount)
        device->write(alignPadding, alignAmount);

    // Data for morphTargets
    for (quint32 i = 0; i < targetBufferEntriesCount; ++i) {
        const Mesh::VertexBufferEntry &entry(mesh.m_targetBuffer.entries[i]);
        const quint32 componentType = quint32(entry.componentType);
        const quint32 componentCount = entry.componentCount;
        const quint32 offset = entry.offset;
        outputStream << quint32(0) // legacy offset
                     << componentType
                     << componentCount
                     << offset;
        entriesByteSize += VERTEX_BUFFER_ENTRY_STRUCT_SIZE;
    }
    alignAmount = offsetTracker.alignedAdvance(entriesByteSize);
    if (alignAmount)
        device->write(alignPadding, alignAmount);

    for (quint32 i = 0; i < targetBufferEntriesCount; ++i) {
        const Mesh::VertexBufferEntry &entry(mesh.m_targetBuffer.entries[i]);
        const quint32 nameLength = entry.name.size() + 1;
        outputStream << nameLength;
        device->write(entry.name.constData(), nameLength); // with zero terminator included
        alignAmount = offsetTracker.alignedAdvance(sizeof(quint32) + nameLength);
        if (alignAmount)
            device->write(alignPadding, alignAmount);
    }

    device->write(mesh.m_targetBuffer.data.constData(), targetBufferDataSize);

    const quint32 endPos = device->pos();
    const quint32 sizeInBytes = endPos - startPos;
    device->seek(endPos);
    return sizeInBytes;
}

Mesh Mesh::loadMesh(QIODevice *device, quint32 id)
{
    MeshInternal::MeshDataHeader header;
    const MeshInternal::MultiMeshInfo meshFileInfo = MeshInternal::readFileHeader(device);
    auto it = meshFileInfo.meshEntries.constFind(id);
    if (it != meshFileInfo.meshEntries.constEnd()) {
        Mesh mesh;
        quint64 size = MeshInternal::readMeshData(device, *it, &mesh, &header);
        if (size)
            return mesh;
    } else if (id == 0 && !meshFileInfo.meshEntries.isEmpty()) {
        Mesh mesh;
        quint64 size = MeshInternal::readMeshData(device, *meshFileInfo.meshEntries.cbegin(), &mesh, &header);
        if (size)
            return mesh;
    }
    return Mesh();
}

QMap<quint32, Mesh> Mesh::loadAll(QIODevice *device)
{
    MeshInternal::MeshDataHeader header;
    const MeshInternal::MultiMeshInfo meshFileInfo = MeshInternal::readFileHeader(device);
    QMap<quint32, Mesh> meshes;
    for (auto it = meshFileInfo.meshEntries.cbegin(), end = meshFileInfo.meshEntries.cend(); it != end; ++it) {
        Mesh mesh;
        quint64 size = MeshInternal::readMeshData(device, *it, &mesh, &header);
        if (size)
            meshes.insert(it.key(), mesh);
        else
            qWarning("Failed to find mesh #%u", it.key());
    }
    return meshes;
}

static inline quint32 getAlignedOffset(quint32 offset, quint32 align)
{
    Q_ASSERT(align > 0);
    const quint32 leftover = (align > 0) ? offset % align : 0;
    if (leftover)
        return offset + (align - leftover);
    return offset;
}

Mesh Mesh::fromAssetData(const QVector<AssetVertexEntry> &vbufEntries,
                         const QByteArray &indexBufferData,
                         ComponentType indexComponentType,
                         const QVector<AssetMeshSubset> &subsets,
                         quint32 numTargets,
                         quint32 numTargetComps)
{
    Mesh mesh;
    quint32 currentOffset = 0;
    quint32 bufferAlignment = 0;
    quint32 numItems = 0;
    bool ok = true;

    mesh.m_targetBuffer.numTargets = numTargets;
    quint32 targetCurrentComp = 0;
    quint32 targetCompStride = 0;

    QVector<AssetVertexEntry> vEntries;
    for (const AssetVertexEntry &entry : vbufEntries) {
        // Ignore entries with no data.
        if (entry.data.isEmpty())
            continue;

        VertexBufferEntry meshEntry;
        meshEntry.componentType = entry.componentType;
        meshEntry.componentCount = entry.componentCount;
        meshEntry.name = entry.name;

        if (entry.morphTargetId < 0) {
            const quint32 alignment = MeshInternal::byteSizeForComponentType(entry.componentType);
            const quint32 byteSize = alignment * entry.componentCount;

            if (entry.data.size() % alignment != 0) {
                Q_ASSERT(false);
                ok = false;
            }

            quint32 localNumItems = entry.data.size() / byteSize;
            if (numItems == 0) {
                numItems = localNumItems;
            } else if (numItems != localNumItems) {
                Q_ASSERT(false);
                ok = false;
                numItems = qMin(numItems, localNumItems);
            }

            currentOffset = getAlignedOffset(currentOffset, alignment);
            meshEntry.offset = currentOffset;

            mesh.m_vertexBuffer.entries.append(meshEntry);
            currentOffset += byteSize;
            bufferAlignment = qMax(bufferAlignment, alignment);
            vEntries.append(entry);
        } else {
            if (!targetCompStride) {
                const quint32 targetEntrySize = entry.data.size();
                quint32 targetEntryTexWidth = qCeil(qSqrt(((targetEntrySize + 15) >> 4)));
                targetCompStride = targetEntryTexWidth * targetEntryTexWidth * 4 * sizeof(float);
                mesh.m_targetBuffer.data.resize(targetCompStride * numTargets * numTargetComps);
            }

            // At assets, these entries are appended sequentially from target 0 to target N - 1
            // It is safe to calculate the offset by the data size
            meshEntry.offset = (targetCurrentComp * numTargets + entry.morphTargetId)
                                    * targetCompStride;
            memcpy(mesh.m_targetBuffer.data.data() + meshEntry.offset,
                   entry.data.constData(), entry.data.size());

            // Note: the targetBuffer will not be interleaved,
            // data will be just appended in order and used for a texture array.
            if (entry.morphTargetId == 0)
                mesh.m_targetBuffer.entries.append(meshEntry);

            targetCurrentComp = (targetCurrentComp + 1 < numTargetComps) ? targetCurrentComp + 1 : 0;
        }
    }

    if (!ok)
        return Mesh();

    mesh.m_vertexBuffer.stride = getAlignedOffset(currentOffset, bufferAlignment);

    // Packed interleave the data.
    for (quint32 idx = 0; idx < numItems; ++idx) {
        quint32 dataOffset = 0;
        for (const AssetVertexEntry &entry : vEntries) {
            if (entry.data.isEmpty())
                continue;

            const quint32 alignment = MeshInternal::byteSizeForComponentType(entry.componentType);
            const quint32 byteSize = alignment * entry.componentCount;
            const quint32 offset = byteSize * idx;
            const quint32 newOffset = getAlignedOffset(dataOffset, alignment);
            if (newOffset != dataOffset) {
                QByteArray filler(newOffset - dataOffset, '\0');
                mesh.m_vertexBuffer.data.append(filler);
            }

            mesh.m_vertexBuffer.data.append(entry.data.constData() + offset, byteSize);
            dataOffset = newOffset + byteSize;
        }
        Q_ASSERT(dataOffset == mesh.m_vertexBuffer.stride);
    }

    mesh.m_indexBuffer.componentType = indexComponentType;
    mesh.m_indexBuffer.data = indexBufferData;

    for (const AssetMeshSubset &subset : subsets) {
        Mesh::Subset meshSubset;
        meshSubset.name = subset.name;
        meshSubset.count = subset.count;
        meshSubset.offset = subset.offset;

        // TODO: QTBUG-102026
        if (subset.boundsPositionEntryIndex != std::numeric_limits<quint32>::max()) {
            const QSSGBounds3 bounds = MeshInternal::calculateSubsetBounds(
                    mesh.m_vertexBuffer.entries[subset.boundsPositionEntryIndex],
                    mesh.m_vertexBuffer.data,
                    mesh.m_vertexBuffer.stride,
                    mesh.m_indexBuffer.data,
                    mesh.m_indexBuffer.componentType,
                    subset.count,
                    subset.offset);
            meshSubset.bounds.min = bounds.minimum;
            meshSubset.bounds.max = bounds.maximum;
        }

        meshSubset.lightmapSizeHint = QSize(subset.lightmapWidth, subset.lightmapHeight);
        meshSubset.lods = subset.lods;

        mesh.m_subsets.append(meshSubset);
    }

    mesh.m_drawMode = DrawMode::Triangles;
    mesh.m_winding = Winding::CounterClockwise;

    return mesh;
}

Mesh Mesh::fromRuntimeData(const RuntimeMeshData &data, QString *error)
{
    if (data.m_vertexBuffer.size() == 0) {
        *error = QObject::tr("Vertex buffer empty");
        return Mesh();
    }
    if (data.m_attributeCount == 0) {
        *error = QObject::tr("No attributes defined");
        return Mesh();
    }

    Mesh mesh;
    mesh.m_drawMode = data.m_primitiveType;
    mesh.m_winding = Winding::CounterClockwise;

    for (int i = 0; i < data.m_attributeCount; ++i) {
        const RuntimeMeshData::Attribute &att = data.m_attributes[i];
        if (att.semantic == RuntimeMeshData::Attribute::IndexSemantic) {
            mesh.m_indexBuffer.componentType = att.componentType;
        } else {
            const char *name = nullptr;
            switch (att.semantic) {
            case RuntimeMeshData::Attribute::PositionSemantic:
                name = MeshInternal::getPositionAttrName();
                break;
            case RuntimeMeshData::Attribute::NormalSemantic:
                name = MeshInternal::getNormalAttrName();
                break;
            case RuntimeMeshData::Attribute::TexCoord0Semantic:
                name = MeshInternal::getUV0AttrName();
                break;
            case RuntimeMeshData::Attribute::TexCoord1Semantic:
                name = MeshInternal::getUV1AttrName();
                break;
            case RuntimeMeshData::Attribute::TangentSemantic:
                name = MeshInternal::getTexTanAttrName();
                break;
            case RuntimeMeshData::Attribute::BinormalSemantic:
                name = MeshInternal::getTexBinormalAttrName();
                break;
            case RuntimeMeshData::Attribute::JointSemantic:
                name = MeshInternal::getJointAttrName();
                break;
            case RuntimeMeshData::Attribute::WeightSemantic:
                name = MeshInternal::getWeightAttrName();
                break;
            case RuntimeMeshData::Attribute::ColorSemantic:
                name = MeshInternal::getColorAttrName();
                break;
            default:
                *error = QObject::tr("Warning: Invalid attribute semantic: %1")
                        .arg(att.semantic);
                return Mesh();
            }

            VertexBufferEntry entry;
            entry.componentType = att.componentType;
            entry.componentCount = att.componentCount();
            entry.offset = att.offset;
            entry.name = name;
            mesh.m_vertexBuffer.entries.append(entry);
        }
    }

    mesh.m_vertexBuffer.data = data.m_vertexBuffer;
    // Only interleaved vertex attribute packing is supported, both internally
    // and in the QQuick3DGeometry API, hence the per-vertex buffer stride.
    mesh.m_vertexBuffer.stride = data.m_stride;
    mesh.m_subsets = data.m_subsets;
    mesh.m_indexBuffer.data = data.m_indexBuffer;

    if (!data.m_targetBuffer.isEmpty()) {
        const quint32 vertexCount = data.m_vertexBuffer.size() / data.m_stride;
        const quint32 targetEntryTexWidth = qCeil(qSqrt(vertexCount));
        const quint32 targetCompStride = targetEntryTexWidth * targetEntryTexWidth * 4 * sizeof(float);
        mesh.m_targetBuffer.data.resize(targetCompStride * data.m_targetAttributeCount);

        QVarLengthArray<RuntimeMeshData::TargetAttribute> sortedAttribs(
                                            data.m_targetAttributes,
                                            data.m_targetAttributes + data.m_targetAttributeCount);
        std::sort(sortedAttribs.begin(), sortedAttribs.end(),
                  [] (RuntimeMeshData::TargetAttribute a, RuntimeMeshData::TargetAttribute b) {
                  return (a.targetId == b.targetId) ? a.attr.semantic < b.attr.semantic :
                                                      a.targetId < b.targetId; });
        for (int i = 0; i < data.m_targetAttributeCount; ++i) {
            const RuntimeMeshData::Attribute &att = sortedAttribs[i].attr;
            const int stride = (sortedAttribs[i].stride < 1) ? att.componentCount() * sizeof(float)
                                                             : sortedAttribs[i].stride;
            const char *name = nullptr;
            switch (att.semantic) {
            case RuntimeMeshData::Attribute::PositionSemantic:
                name = MeshInternal::getPositionAttrName();
                break;
            case RuntimeMeshData::Attribute::NormalSemantic:
                name = MeshInternal::getNormalAttrName();
                break;
            case RuntimeMeshData::Attribute::TexCoord0Semantic:
                name = MeshInternal::getUV0AttrName();
                break;
            case RuntimeMeshData::Attribute::TexCoord1Semantic:
                name = MeshInternal::getUV1AttrName();
                break;
            case RuntimeMeshData::Attribute::TangentSemantic:
                name = MeshInternal::getTexTanAttrName();
                break;
            case RuntimeMeshData::Attribute::BinormalSemantic:
                name = MeshInternal::getTexBinormalAttrName();
                break;
            case RuntimeMeshData::Attribute::IndexSemantic:
            case RuntimeMeshData::Attribute::JointSemantic:
            case RuntimeMeshData::Attribute::WeightSemantic:
                *error = QObject::tr("Warning: Invalid target attribute semantic: %1")
                        .arg(att.semantic);
                continue;
            case RuntimeMeshData::Attribute::ColorSemantic:
                name = MeshInternal::getColorAttrName();
                break;
            default:
                *error = QObject::tr("Warning: Invalid target attribute semantic: %1")
                        .arg(att.semantic);
                return Mesh();
            }
            char *dstBuf = mesh.m_targetBuffer.data.data() + i * targetCompStride;
            const char *srcBuf = data.m_targetBuffer.constData() + att.offset;
            Q_ASSERT(att.componentType == Mesh::ComponentType::Float32);
            if (stride == 4 * sizeof(float)) {
                memcpy(dstBuf, srcBuf, vertexCount * stride);
            } else {
                for (quint32 j = 0; j < vertexCount; ++j) {
                    memcpy(dstBuf + j * 4 * sizeof(float),
                           srcBuf + j * stride,
                           att.componentCount() * sizeof(float));
                }
            }

            if (sortedAttribs[i].targetId == 0) {
                VertexBufferEntry entry;
                entry.componentType = att.componentType;
                entry.componentCount = att.componentCount();
                entry.offset = i * targetCompStride;
                entry.name = name;
                mesh.m_targetBuffer.entries.append(entry);
            }
        }
        mesh.m_targetBuffer.numTargets = data.m_targetAttributeCount / mesh.m_targetBuffer.entries.size();
    }
    return mesh;
}

quint32 Mesh::save(QIODevice *device, quint32 id) const
{
    qint64 newMeshStartPosFromEnd = 0;
    quint32 newId = 1;
    MeshInternal::MultiMeshInfo header;

    if (device->size() > 0) {
        header = MeshInternal::readFileHeader(device);
        if (!header.isValid()) {
            qWarning("There is existing data, but mesh file header is invalid; cannot save");
            return 0;
        }
        for (auto it = header.meshEntries.cbegin(), end = header.meshEntries.cend(); it != end; ++it) {
            if (id) {
                Q_ASSERT(id != it.key());
                newId = id;
            } else {
                newId = qMax(newId, it.key() + 1);
            }
        }
        newMeshStartPosFromEnd = MULTI_HEADER_STRUCT_SIZE + header.meshEntries.size() * MULTI_ENTRY_STRUCT_SIZE;
    } else {
        header = MeshInternal::MultiMeshInfo::withDefaults();
    }

    // the new mesh data overwrites the entry list and file header
    device->seek(device->size() - newMeshStartPosFromEnd);
    const qint64 meshOffset = device->pos();
    header.meshEntries.insert(newId, meshOffset);

    MeshInternal::MeshDataHeader meshHeader = MeshInternal::MeshDataHeader::withDefaults();
    // skip the space for the mesh header for now
    device->seek(device->pos() + MESH_HEADER_STRUCT_SIZE);
    meshHeader.sizeInBytes = MeshInternal::writeMeshData(device, *this);
    // now the mesh header is ready to be written out
    device->seek(meshOffset);
    MeshInternal::writeMeshHeader(device, meshHeader);
    device->seek(meshOffset + MESH_HEADER_STRUCT_SIZE + meshHeader.sizeInBytes);
    // write out new entry list and file header
    MeshInternal::writeFileHeader(device, header);

    return newId;
}

QSSGBounds3 MeshInternal::calculateSubsetBounds(const Mesh::VertexBufferEntry &entry,
                                                const QByteArray &vertexBufferData,
                                                quint32 vertexBufferStride,
                                                const QByteArray &indexBufferData,
                                                Mesh::ComponentType indexComponentType,
                                                quint32 subsetCount,
                                                quint32 subsetOffset)
{
    QSSGBounds3 result;
    if (entry.componentType != Mesh::ComponentType::Float32 || entry.componentCount != 3) {
        Q_ASSERT(false);
        return result;
    }

    const int indexComponentByteSize = byteSizeForComponentType(indexComponentType);
    if (indexComponentByteSize != 2 && indexComponentByteSize != 4) {
        Q_ASSERT(false);
        return result;
    }

    const quint32 indexBufferCount = indexBufferData.size() / indexComponentByteSize;
    const quint32 vertexBufferByteSize = vertexBufferData.size();
    const char *vertexSrcPtr = vertexBufferData.constData();
    const char *indexSrcPtr = indexBufferData.constData();

    for (quint32 idx = 0, numItems = subsetCount; idx < numItems; ++idx) {
        if (idx + subsetOffset >= indexBufferCount)
            continue;

        quint32 vertexIdx = 0;
        switch (indexComponentByteSize) {
        case 2:
            vertexIdx = reinterpret_cast<const quint16 *>(indexSrcPtr)[idx + subsetOffset];
            break;
        case 4:
            vertexIdx = reinterpret_cast<const quint32 *>(indexSrcPtr)[idx + subsetOffset];
            break;
        default:
            Q_UNREACHABLE();
            break;
        }

        const quint32 finalOffset = entry.offset + (vertexIdx * vertexBufferStride);
        float v[3];
        if (finalOffset + sizeof(v) <= vertexBufferByteSize) {
            memcpy(v, vertexSrcPtr + finalOffset, sizeof(v));
            result.include(QVector3D(v[0], v[1], v[2]));
        } else {
            Q_ASSERT(false);
        }
    }

    return result;
}

bool Mesh::hasLightmapUVChannel() const
{
    const char *lightmapAttrName = MeshInternal::getLightmapUVAttrName();
    for (const VertexBufferEntry &vbe : std::as_const(m_vertexBuffer.entries)) {
        if (vbe.name == lightmapAttrName)
            return true;
    }
    return false;
}

bool Mesh::createLightmapUVChannel(float texelsPerUnit, const QMatrix4x4 &scale)
{
    const char *posAttrName = MeshInternal::getPositionAttrName();
    const char *normalAttrName = MeshInternal::getNormalAttrName();
    const char *uvAttrName = MeshInternal::getUV0AttrName();
    const char *lightmapAttrName = MeshInternal::getLightmapUVAttrName();

    // this function should do nothing if there is already an attr_lightmapuv
    if (hasLightmapUVChannel())
        return true;

    const char *srcVertexData = m_vertexBuffer.data.constData();
    const quint32 srcVertexStride = m_vertexBuffer.stride;
    if (!srcVertexStride) {
        qWarning("Lightmap UV unwrapping encountered a Mesh with 0 vertex stride, this cannot happen");
        return false;
    }
    if (m_indexBuffer.data.isEmpty()) {
        qWarning("Lightmap UV unwrapping encountered a Mesh without index data, this cannot happen");
        return false;
    }

    quint32 positionOffset = UINT32_MAX;
    quint32 normalOffset = UINT32_MAX;
    quint32 uvOffset = UINT32_MAX;

    for (const VertexBufferEntry &vbe : std::as_const(m_vertexBuffer.entries)) {
        if (vbe.name == posAttrName) {
            if (vbe.componentCount != 3) {
                qWarning("Lightmap UV unwrapping encountered a Mesh non-float3 position data, this cannot happen");
                return false;
            }
            positionOffset = vbe.offset;
        } else if (vbe.name == normalAttrName) {
            if (vbe.componentCount != 3) {
                qWarning("Lightmap UV unwrapping encountered a Mesh non-float3 normal data, this cannot happen");
                return false;
            }
            normalOffset = vbe.offset;
        } else if (vbe.name == uvAttrName) {
            if (vbe.componentCount != 2) {
                qWarning("Lightmap UV unwrapping encountered a Mesh non-float2 UV0 data, this cannot happen");
                return false;
            }
            uvOffset = vbe.offset;
        }
    }

    if (positionOffset == UINT32_MAX) {
        qWarning("Lightmap UV unwrapping encountered a Mesh without vertex positions, this cannot happen");
        return false;
    }
    // normal and uv0 are optional

    const qsizetype vertexCount = m_vertexBuffer.data.size() / srcVertexStride;
    QByteArray positionData(vertexCount * 3 * sizeof(float), Qt::Uninitialized);
    float *posPtr = reinterpret_cast<float *>(positionData.data());
    for (qsizetype i = 0; i < vertexCount; ++i) {
        const char *vertexBasePtr = srcVertexData + i * srcVertexStride;
        const float *srcPos = reinterpret_cast<const float *>(vertexBasePtr + positionOffset);
        QVector3D srcV;
        srcV.setX(*srcPos++);
        srcV.setY(*srcPos++);
        srcV.setZ(*srcPos++);
        // We scale the positions here, but not on the source mesh, so that the uv unwrapper works on
        // the positions that the model will have in the scene after its scaling has been applied. This
        // way the texels-per-unit will be correct.
        srcV = scale.map(srcV);
        *posPtr++ = srcV.x();
        *posPtr++ = srcV.y();
        *posPtr++ = srcV.z();
    }

    QByteArray normalData;
    if (normalOffset != UINT32_MAX) {
        normalData.resize(vertexCount * 3 * sizeof(float));
        float *normPtr = reinterpret_cast<float *>(normalData.data());
        for (qsizetype i = 0; i < vertexCount; ++i) {
            const char *vertexBasePtr = srcVertexData + i * srcVertexStride;
            const float *srcNormal = reinterpret_cast<const float *>(vertexBasePtr + normalOffset);
            *normPtr++ = *srcNormal++;
            *normPtr++ = *srcNormal++;
            *normPtr++ = *srcNormal++;
        }
    }

    QByteArray uvData;
    if (uvOffset != UINT32_MAX) {
        uvData.resize(vertexCount * 2 * sizeof(float));
        float *uvPtr = reinterpret_cast<float *>(uvData.data());
        for (qsizetype i = 0; i < vertexCount; ++i) {
            const char *vertexBasePtr = srcVertexData + i * srcVertexStride;
            const float *srcUv = reinterpret_cast<const float *>(vertexBasePtr + uvOffset);
            *uvPtr++ = *srcUv++;
            *uvPtr++ = *srcUv++;
        }
    }

    QSSGLightmapUVGenerator uvGen;
    QSSGLightmapUVGeneratorResult r = uvGen.run(positionData, normalData, uvData,
                                                m_indexBuffer.data, m_indexBuffer.componentType,
                                                texelsPerUnit);
    if (!r.isValid())
        return false;

    // the result can have more (but never less) vertices than the input
    const int newVertexCount = r.vertexMap.size();

    // r.indexData contains the new index data that has the same number of elements as before
    const quint32 *newIndex = reinterpret_cast<const quint32 *>(r.indexData.constData());
    if (m_indexBuffer.componentType == QSSGMesh::Mesh::ComponentType::UnsignedInt32) {
        if (r.indexData.size() != m_indexBuffer.data.size()) {
            qWarning("Index buffer size mismatch after lightmap UV unwrapping");
            return false;
        }
        quint32 *indexDst = reinterpret_cast<quint32 *>(m_indexBuffer.data.data());
        memcpy(indexDst, newIndex, m_indexBuffer.data.size());
    } else {
        if (r.indexData.size() != m_indexBuffer.data.size() * 2) {
            qWarning("Index buffer size mismatch after lightmap UV unwrapping");
            return false;
        }
        quint16 *indexDst = reinterpret_cast<quint16 *>(m_indexBuffer.data.data());
        for (size_t i = 0, count = m_indexBuffer.data.size() / sizeof(quint16); i != count; ++i)
            *indexDst++ = *newIndex++;
    }

    QVarLengthArray<QByteArray, 8> newData;
    newData.reserve(m_vertexBuffer.entries.size());

    for (const VertexBufferEntry &vbe : std::as_const(m_vertexBuffer.entries)) {
        const qsizetype byteSize = vbe.componentCount * MeshInternal::byteSizeForComponentType(vbe.componentType);
        QByteArray data(byteSize * vertexCount, Qt::Uninitialized);
        char *dst = data.data();
        for (qsizetype i = 0; i < vertexCount; ++i) {
            memcpy(dst, srcVertexData + i * srcVertexStride + vbe.offset, byteSize);
            dst += byteSize;
        }
        switch (vbe.componentType) {
        case ComponentType::UnsignedInt8:
            newData.append(QSSGLightmapUVGenerator::remap<quint8>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::Int8:
            newData.append(QSSGLightmapUVGenerator::remap<qint8>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::UnsignedInt16:
            newData.append(QSSGLightmapUVGenerator::remap<quint16>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::Int16:
            newData.append(QSSGLightmapUVGenerator::remap<qint16>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::UnsignedInt32:
            newData.append(QSSGLightmapUVGenerator::remap<quint32>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::Int32:
            newData.append(QSSGLightmapUVGenerator::remap<qint32>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::UnsignedInt64:
            newData.append(QSSGLightmapUVGenerator::remap<quint64>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::Int64:
            newData.append(QSSGLightmapUVGenerator::remap<qint64>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::Float16:
            newData.append(QSSGLightmapUVGenerator::remap<qfloat16>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::Float32:
            newData.append(QSSGLightmapUVGenerator::remap<float>(data, r.vertexMap, vbe.componentCount));
            break;
        case ComponentType::Float64:
            newData.append(QSSGLightmapUVGenerator::remap<double>(data, r.vertexMap, vbe.componentCount));
            break;
        }
    }

    VertexBufferEntry lightmapUVEntry;
    lightmapUVEntry.componentType = ComponentType::Float32;
    lightmapUVEntry.componentCount = 2;
    lightmapUVEntry.offset = 0;
    lightmapUVEntry.name = lightmapAttrName;

    QByteArray newVertexBuffer;
    newVertexBuffer.reserve(newVertexCount * (srcVertexStride + 8));

    quint32 bufferAlignment = 0;
    for (int vertexIdx = 0; vertexIdx < newVertexCount; ++vertexIdx) {
        quint32 dataOffset = 0;
        for (int vbIdx = 0, end = m_vertexBuffer.entries.size(); vbIdx != end; ++vbIdx) {
            VertexBufferEntry &vbe(m_vertexBuffer.entries[vbIdx]);

            const quint32 alignment = MeshInternal::byteSizeForComponentType(vbe.componentType);
            bufferAlignment = qMax(bufferAlignment, alignment);
            const quint32 byteSize = alignment * vbe.componentCount;
            const quint32 newOffset = getAlignedOffset(dataOffset, alignment);

            if (newOffset != dataOffset) {
                QByteArray filler(newOffset - dataOffset, '\0');
                newVertexBuffer.append(filler);
            }

            if (vertexIdx == 0)
                vbe.offset = newVertexBuffer.size();

            newVertexBuffer.append(newData[vbIdx].constData() + byteSize * vertexIdx, byteSize);
            dataOffset = newOffset + byteSize;
        }

        const quint32 byteSize = 2 * sizeof(float);
        const quint32 newOffset = getAlignedOffset(dataOffset, byteSize);
        if (newOffset != dataOffset) {
            QByteArray filler(newOffset - dataOffset, '\0');
            newVertexBuffer.append(filler);
        }

        if (vertexIdx == 0)
            lightmapUVEntry.offset = newVertexBuffer.size();

        newVertexBuffer.append(r.lightmapUVChannel.constData() + byteSize * vertexIdx, byteSize);
        dataOffset = newOffset + byteSize;

        if (vertexIdx == 0)
            m_vertexBuffer.stride = getAlignedOffset(dataOffset, bufferAlignment);
    }

    m_vertexBuffer.entries.append(lightmapUVEntry);

    m_vertexBuffer.data = newVertexBuffer;

    const QSize lightmapSizeHint(r.lightmapWidth, r.lightmapHeight);
    for (Subset &subset : m_subsets)
        subset.lightmapSizeHint = lightmapSizeHint;

    return true;
}

size_t simplifyMesh(unsigned int *destination, const unsigned int *indices, size_t indexCount, const float *vertexPositions, size_t vertexCount, size_t vertexPositionsStride, size_t targetIndexCount, float targetError, unsigned int options, float *resultError)
{
    return meshopt_simplify(destination, indices, indexCount, vertexPositions, vertexCount, vertexPositionsStride, targetIndexCount, targetError, options, resultError);
}

float simplifyScale(const float *vertexPositions, size_t vertexCount, size_t vertexPositionsStride)
{
    return meshopt_simplifyScale(vertexPositions, vertexCount, vertexPositionsStride);
}

void optimizeVertexCache(unsigned int *destination, const unsigned int *indices, size_t indexCount, size_t vertexCount)
{
    meshopt_optimizeVertexCache(destination, indices, indexCount, vertexCount);
}

} // namespace QSSGMesh

QT_END_NAMESPACE

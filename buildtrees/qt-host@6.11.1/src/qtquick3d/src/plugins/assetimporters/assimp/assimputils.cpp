// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "assimputils.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/importerdesc.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtCore/qstring.h>
#include <QtCore/QHash>
#include <QtCore/QSet>

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

QT_BEGIN_NAMESPACE

namespace
{

struct SubsetEntryData {
    QString name;
    int indexLength;
    int indexOffset;
    quint32 lightmapWidth;
    quint32 lightmapHeight;
    QVector<QSSGMesh::Mesh::Lod> lods;
};

struct IntVector4D {
    qint32 x = 0;
    qint32 y = 0;
    qint32 z = 0;
    qint32 w = 0;
};

struct VertexAttributeData {
    QVector3D position;
    QVector3D normal;
    QVector3D uv0;
    QVector3D uv1;
    QVector3D tangent;
    QVector3D binormal;
    QVector4D color;
};

struct VertexAttributeDataExt {
    VertexAttributeData aData;
    IntVector4D boneIndexes;
    QVector4D boneWeights;
    QVector<VertexAttributeData> targetAData;
};

struct VertexDataRequirments {
    bool needsPositionData = false;
    bool needsNormalData = false;
    bool needsTangentData = false;
    bool needsVertexColorData = false;
    unsigned uv0Components = 0;
    unsigned uv1Components = 0;
    bool needsUV0Data = false;
    bool needsUV1Data = false;
    bool needsBones = false;
    bool useFloatJointIndices = false;

    quint32 numMorphTargets = 0;
    // All the target mesh will have the same components
    // Target texture coords will be recored as 3 components.
    // even if we are using just 2 components now.
    bool needsTargetPositionData = false;
    bool needsTargetNormalData = false;
    bool needsTargetTangentData = false;
    bool needsTargetVertexColorData = false;
    bool needsTargetUV0Data = false;
    bool needsTargetUV1Data = false;

    void collectRequirmentsForMesh(const aiMesh *mesh) {
        uv0Components = qMax(mesh->mNumUVComponents[0], uv0Components);
        uv1Components = qMax(mesh->mNumUVComponents[1], uv1Components);
        needsUV0Data |= mesh->HasTextureCoords(0);
        needsUV1Data |= mesh->HasTextureCoords(1);
        needsPositionData |= mesh->HasPositions();
        needsNormalData |= mesh->HasNormals();
        needsTangentData |= mesh->HasTangentsAndBitangents();
        needsVertexColorData |=mesh->HasVertexColors(0);
        needsBones |= mesh->HasBones();
        numMorphTargets = mesh->mNumAnimMeshes;
        if (numMorphTargets && mesh->mAnimMeshes) {
            for (uint i = 0; i < numMorphTargets; ++i) {
                auto animMesh = mesh->mAnimMeshes[i];
                needsTargetPositionData |= animMesh->HasPositions();
                needsTargetNormalData |= animMesh->HasNormals();
                needsTargetTangentData |= animMesh->HasTangentsAndBitangents();
                needsTargetVertexColorData |= animMesh->HasVertexColors(0);
                needsTargetUV0Data |= animMesh->HasTextureCoords(0);
                needsTargetUV1Data |= animMesh->HasTextureCoords(1);
            }
        }
    }
};

QVector<VertexAttributeDataExt> getVertexAttributeData(const aiMesh *mesh, const VertexDataRequirments &requirments)
{
    QVector<VertexAttributeDataExt> vertexAttributes;

    vertexAttributes.resize(mesh->mNumVertices);

    // Positions
    if (mesh->HasPositions()) {
        for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
            const auto vertex = mesh->mVertices[index];
            vertexAttributes[index].aData.position = QVector3D(vertex.x, vertex.y, vertex.z);
        }
    }

    // Normals
    if (mesh->HasNormals()) {
        for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
            const auto normal = mesh->mNormals[index];
            vertexAttributes[index].aData.normal = QVector3D(normal.x, normal.y, normal.z);
        }
    }

    // UV0
    if (mesh->HasTextureCoords(0)) {
        const auto texCoords = mesh->mTextureCoords[0];
        if (requirments.uv0Components == 2) {
            for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
                const auto uv = texCoords[index];
                vertexAttributes[index].aData.uv0 = QVector3D(uv.x, uv.y, 0.0f);
            }
        } else if (requirments.uv0Components == 3) {
            for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
                const auto uv = texCoords[index];
                vertexAttributes[index].aData.uv0 = QVector3D(uv.x, uv.y, uv.z);
            }
        }
    }

    // UV1
    if (mesh->HasTextureCoords(1)) {
        const auto texCoords = mesh->mTextureCoords[1];
        if (requirments.uv1Components == 2) {
            for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
                const auto uv = texCoords[index];
                vertexAttributes[index].aData.uv1 = QVector3D(uv.x, uv.y, 0.0f);
            }
        } else if (requirments.uv1Components == 3) {
            for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
                const auto uv = texCoords[index];
                vertexAttributes[index].aData.uv1 = QVector3D(uv.x, uv.y, uv.z);
            }
        }
    }

    // Tangents and Binormals
    if (mesh->HasTangentsAndBitangents()) {
        for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
            const auto tangent = mesh->mTangents[index];
            const auto binormal = mesh->mBitangents[index];
            vertexAttributes[index].aData.tangent = QVector3D(tangent.x, tangent.y, tangent.z);
            vertexAttributes[index].aData.binormal = QVector3D(binormal.x, binormal.y, binormal.z);
        }
    }

    // Vertex Colors
    if (mesh->HasVertexColors(0)) {
        for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
            const auto color = mesh->mColors[0][index];
            vertexAttributes[index].aData.color = QVector4D(color.r, color.g, color.b, color.a);
        }
    }

    // Bones + Weights
    if (mesh->HasBones()) {
        for (uint i = 0; i < mesh->mNumBones; ++i) {
            const uint vId = i;
            for (uint j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
                quint32 vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
                float weight = mesh->mBones[i]->mWeights[j].mWeight;

                // skip a bone transform having small weight
                if (weight <= 0.01f)
                    continue;

                //  if any vertex has more weights than 4, it will be ignored
                if (vertexAttributes[vertexId].boneWeights.x() == 0.0f) {
                    vertexAttributes[vertexId].boneIndexes.x = qint32(vId);
                    vertexAttributes[vertexId].boneWeights.setX(weight);
                } else if (vertexAttributes[vertexId].boneWeights.y() == 0.0f) {
                    vertexAttributes[vertexId].boneIndexes.y = qint32(vId);
                    vertexAttributes[vertexId].boneWeights.setY(weight);
                } else if (vertexAttributes[vertexId].boneWeights.z() == 0.0f) {
                    vertexAttributes[vertexId].boneIndexes.z = qint32(vId);
                    vertexAttributes[vertexId].boneWeights.setZ(weight);
                } else if (vertexAttributes[vertexId].boneWeights.w() == 0.0f) {
                    vertexAttributes[vertexId].boneIndexes.w = qint32(vId);
                    vertexAttributes[vertexId].boneWeights.setW(weight);
                } else {
                    qWarning("vertexId %d has already 4 weights and index %d's weight %f will be ignored.", vertexId, vId, weight);
                }
            }
        }
    }

    // Morph Targets
    if (requirments.numMorphTargets > 0) {
        for (unsigned int index = 0; index < mesh->mNumVertices; ++index) {
            vertexAttributes[index].targetAData.resize(requirments.numMorphTargets);

            for (uint i = 0; i < requirments.numMorphTargets; ++i) {
                if (i >= mesh->mNumAnimMeshes)
                    continue;

                auto animMesh = mesh->mAnimMeshes[i];
                if (animMesh->HasPositions()) {
                    const auto vertex = animMesh->mVertices[index];
                    vertexAttributes[index].targetAData[i].position = QVector3D(vertex.x, vertex.y, vertex.z);
                }
                if (animMesh->HasNormals()) {
                    const auto normal = animMesh->mNormals[index];
                    vertexAttributes[index].targetAData[i].normal = QVector3D(normal.x, normal.y, normal.z);
                }
                if (animMesh->HasTangentsAndBitangents()) {
                    const auto tangent = animMesh->mTangents[index];
                    const auto binormal = animMesh->mBitangents[index];
                    vertexAttributes[index].targetAData[i].tangent = QVector3D(tangent.x, tangent.y, tangent.z);
                    vertexAttributes[index].targetAData[i].binormal = QVector3D(binormal.x, binormal.y, binormal.z);
                }
                if (animMesh->HasTextureCoords(0)) {
                    const auto texCoords = animMesh->mTextureCoords[0];
                    const auto uv = texCoords[index];
                    vertexAttributes[index].targetAData[i].uv0 = QVector3D(uv.x, uv.y, uv.z);
                }
                if (animMesh->HasTextureCoords(1)) {
                    const auto texCoords = animMesh->mTextureCoords[1];
                    const auto uv = texCoords[index];
                    vertexAttributes[index].targetAData[i].uv1 = QVector3D(uv.x, uv.y, uv.z);
                }
                if (animMesh->HasVertexColors(0)) {
                    const auto color = animMesh->mColors[0][index];
                    vertexAttributes[index].targetAData[i].color = QVector4D(color.r, color.g, color.b, color.a);
                }
            }
        }
    }

    return vertexAttributes;
}

struct VertexBufferData {
    QByteArray positionData;
    QByteArray normalData;
    QByteArray uv0Data;
    QByteArray uv1Data;
    QByteArray tangentData;
    QByteArray binormalData;
    QByteArray vertexColorData;
};

struct VertexBufferDataExt {
    VertexBufferData vData;
    QByteArray boneIndexData;
    QByteArray boneWeightData;
    QVector<VertexBufferData> targetVData;

    void addVertexAttributeData(const VertexAttributeDataExt &vertex, const VertexDataRequirments &requirments)
    {
        // Position
        if (requirments.needsPositionData)
            vData.positionData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.aData.position), sizeof(QVector3D));
        // Normal
        if (requirments.needsNormalData)
            vData.normalData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.aData.normal), sizeof(QVector3D));
        // UV0

        if (requirments.needsUV0Data) {
            if (requirments.uv0Components == 2) {
                const QVector2D uv(vertex.aData.uv0.x(), vertex.aData.uv0.y());
                vData.uv0Data += QByteArray::fromRawData(reinterpret_cast<const char *>(&uv), sizeof(QVector2D));
            } else {
                vData.uv0Data += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.aData.uv0), sizeof(QVector3D));
            }
        }

        // UV1
        if (requirments.needsUV1Data) {
            if (requirments.uv1Components == 2) {
                const QVector2D uv(vertex.aData.uv1.x(), vertex.aData.uv1.y());
                vData.uv1Data += QByteArray::fromRawData(reinterpret_cast<const char *>(&uv), sizeof(QVector2D));
            } else {
                vData.uv1Data += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.aData.uv1), sizeof(QVector3D));
            }
        }

        // Tangent
        // Binormal
        if (requirments.needsTangentData) {
            vData.tangentData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.aData.tangent), sizeof(QVector3D));
            vData.binormalData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.aData.binormal), sizeof(QVector3D));
        }

        // Color
        if (requirments.needsVertexColorData)
            vData.vertexColorData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.aData.color), sizeof(QVector4D));

        // Bone Indexes
        // Bone Weights
        if (requirments.needsBones) {
            if (requirments.useFloatJointIndices) {
                const QVector4D fBoneIndex(float(vertex.boneIndexes.x), float(vertex.boneIndexes.y), float(vertex.boneIndexes.z), float(vertex.boneIndexes.w));
                boneIndexData += QByteArray::fromRawData(reinterpret_cast<const char *>(&fBoneIndex), sizeof(QVector4D));
            } else {
                boneIndexData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.boneIndexes), sizeof(IntVector4D));
            }
            boneWeightData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.boneWeights), sizeof(QVector4D));
        }

        // Morph Targets
        for (uint i = 0; i < requirments.numMorphTargets; ++i) {
            if (requirments.needsTargetPositionData) {
                targetVData[i].positionData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.targetAData[i].position), sizeof(QVector3D));
                targetVData[i].positionData.append(sizeof(float), '\0');
            }
            if (requirments.needsTargetNormalData) {
                targetVData[i].normalData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.targetAData[i].normal), sizeof(QVector3D));
                targetVData[i].normalData.append(sizeof(float), '\0');
            }
            if (requirments.needsTargetTangentData) {
                targetVData[i].tangentData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.targetAData[i].tangent), sizeof(QVector3D));
                targetVData[i].tangentData.append(sizeof(float), '\0');
                targetVData[i].binormalData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.targetAData[i].binormal), sizeof(QVector3D));
                targetVData[i].binormalData.append(sizeof(float), '\0');
            }
            if (requirments.needsTargetUV0Data) {
                targetVData[i].uv0Data += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.targetAData[i].uv0), sizeof(QVector3D));
                targetVData[i].uv0Data.append(sizeof(float), '\0');
            }
            if (requirments.needsTargetUV1Data) {
                targetVData[i].uv1Data += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.targetAData[i].uv1), sizeof(QVector3D));
                targetVData[i].uv1Data.append(sizeof(float), '\0');
            }
            if (requirments.needsTargetVertexColorData) {
                targetVData[i].vertexColorData += QByteArray::fromRawData(reinterpret_cast<const char *>(&vertex.targetAData[i].color), sizeof(QVector4D));
            }
        }
    }

    QVector<QSSGMesh::AssetVertexEntry> createEntries(const VertexDataRequirments &requirments) {
        QVector<QSSGMesh::AssetVertexEntry> entries;
        if (vData.positionData.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getPositionAttrName(),
                               vData.positionData,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               3
                           });
        }
        if (vData.normalData.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getNormalAttrName(),
                               vData.normalData,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               3
                           });
        }
        if (vData.uv0Data.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getUV0AttrName(),
                               vData.uv0Data,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               requirments.uv0Components
                           });
        }
        if (vData.uv1Data.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getUV1AttrName(),
                               vData.uv1Data,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               requirments.uv1Components
                           });
        }

        if (vData.tangentData.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getTexTanAttrName(),
                               vData.tangentData,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               3
                           });
        }

        if (vData.binormalData.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getTexBinormalAttrName(),
                               vData.binormalData,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               3
                           });
        }

        if (vData.vertexColorData.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getColorAttrName(),
                               vData.vertexColorData,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               4
                           });
        }

        if (boneIndexData.size() > 0) {
            entries.append({
                               QSSGMesh::MeshInternal::getJointAttrName(),
                               boneIndexData,
                               requirments.useFloatJointIndices ?  QSSGMesh::Mesh::ComponentType::Float32 : QSSGMesh::Mesh::ComponentType::Int32,
                               4
                           });
            entries.append({
                               QSSGMesh::MeshInternal::getWeightAttrName(),
                               boneWeightData,
                               QSSGMesh::Mesh::ComponentType::Float32,
                               4
                           });
        }
        for (int i = 0; i < int(requirments.numMorphTargets); ++i) {
            if (targetVData[i].positionData.size() > 0) {
                entries.append({
                                   QSSGMesh::MeshInternal::getPositionAttrName(),
                                   targetVData[i].positionData,
                                   QSSGMesh::Mesh::ComponentType::Float32,
                                   3,
                                   i
                               });
            }
            if (targetVData[i].normalData.size() > 0) {
                entries.append({
                                   QSSGMesh::MeshInternal::getNormalAttrName(),
                                   targetVData[i].normalData,
                                   QSSGMesh::Mesh::ComponentType::Float32,
                                   3,
                                   i
                               });
            }
            if (targetVData[i].tangentData.size() > 0) {
                entries.append({
                                   QSSGMesh::MeshInternal::getTexTanAttrName(),
                                   targetVData[i].tangentData,
                                   QSSGMesh::Mesh::ComponentType::Float32,
                                   3,
                                   i
                               });
            }
            if (targetVData[i].binormalData.size() > 0) {
                entries.append({
                                   QSSGMesh::MeshInternal::getTexBinormalAttrName(),
                                   targetVData[i].binormalData,
                                   QSSGMesh::Mesh::ComponentType::Float32,
                                   3,
                                   i
                               });
            }
            if (targetVData[i].uv0Data.size() > 0) {
                entries.append({
                                   QSSGMesh::MeshInternal::getUV0AttrName(),
                                   targetVData[i].uv0Data,
                                   QSSGMesh::Mesh::ComponentType::Float32,
                                   3,
                                   i
                               });
            }
            if (targetVData[i].uv1Data.size() > 0) {
                entries.append({
                                   QSSGMesh::MeshInternal::getUV1AttrName(),
                                   targetVData[i].uv1Data,
                                   QSSGMesh::Mesh::ComponentType::Float32,
                                   3,
                                   i
                               });
            }
            if (targetVData[i].vertexColorData.size() > 0) {
                entries.append({
                                   QSSGMesh::MeshInternal::getColorAttrName(),
                                   targetVData[i].vertexColorData,
                                   QSSGMesh::Mesh::ComponentType::Float32,
                                   4,
                                   i
                               });
            }
        }
        return entries;
    }
};

QVector<QPair<float, QVector<quint32>>> generateMeshLevelsOfDetail(QVector<VertexAttributeDataExt> &vertexAttributes, QVector<quint32> &indexes, float normalMergeAngle = 60.0f, float normalSplitAngle = 25.0f)
{
    // If both normalMergeAngle and normalSplitAngle are 0.0, then don't recalculate normals
    const bool recalculateNormals = !(qFuzzyIsNull(normalMergeAngle) && qFuzzyIsNull(normalSplitAngle));
    const float normalMergeThreshold = qCos(qDegreesToRadians(normalMergeAngle));
    const float normalSplitThreshold = qCos(qDegreesToRadians(normalSplitAngle));

    QVector<QVector3D> positions;
    positions.reserve(vertexAttributes.size());
    QVector<QVector3D> normals;
    normals.reserve(vertexAttributes.size());
    for (const auto &vertex : vertexAttributes) {
        positions.append(vertex.aData.position);
        normals.append(vertex.aData.normal);
    }

    QVector<QVector3D> splitVertexNormals;
    QVector<quint32> splitVertexIndices;
    quint32 splitVertexCount = vertexAttributes.size();

    const float targetError = std::numeric_limits<float>::max(); // error doesn't matter, index count is more important
    const float *vertexData = reinterpret_cast<const float *>(positions.constData());
    const float scaleFactor = QSSGMesh::simplifyScale(vertexData, positions.size(), sizeof(QVector3D));
    const quint32 indexCount = indexes.size();
    quint32 indexTarget = 12;
    quint32 lastIndexCount = 0;
    QVector<QPair<float, QVector<quint32>>> lods;

    while (indexTarget < indexCount) {
        float error;
        QVector<quint32> newIndexes;
        newIndexes.resize(indexCount); // Must be the same size as the original indexes to pass to simplifyMesh
        size_t newLength = QSSGMesh::simplifyMesh(newIndexes.data(), indexes.constData(), indexes.size(), vertexData, positions.size(), sizeof(QVector3D), indexTarget, targetError, 0, &error);

        // Not good enough, try again
        if (newLength < lastIndexCount * 1.5f) {
            indexTarget = indexTarget * 1.5f;
            continue;
        }

        // We are done
        if (newLength == 0 || (newLength >= (indexCount * 0.75f)))
            break;

        newIndexes.resize(newLength);

        // LOD Normal Correction
        if (recalculateNormals) {
            // Cull any new degenerate triangles and get the new face normals
            QVector<QVector3D> faceNormals;
            {
                QVector<quint32> culledIndexes;
                for (quint32 j = 0; j < newIndexes.size(); j += 3) {
                    const QVector3D &v0 = positions[newIndexes[j]];
                    const QVector3D &v1 = positions[newIndexes[j + 1]];
                    const QVector3D &v2 = positions[newIndexes[j + 2]];

                    QVector3D faceNormal = QVector3D::crossProduct(v1 - v0, v2 - v0);
                    // This normalizes the vector in place and returns the magnitude
                    const float faceArea = QSSGUtils::vec3::normalize(faceNormal);
                    // It is possible that the simplifyMesh process gave us a degenerate triangle
                    // (all three at the same point, or on the same line) or such a small triangle
                    // that a float value doesn't have enough resolution. In that case cull the
                    // "face" since it would not get rendered in a meaningful way anyway
                    if (faceArea != 0.0f) {
                        faceNormals.append(faceNormal);
                        faceNormals.append(faceNormal);
                        faceNormals.append(faceNormal);
                        culledIndexes.append({newIndexes[j], newIndexes[j + 1], newIndexes[j + 2]});
                    }
                }

                if (newIndexes.size() != culledIndexes.size())
                    newIndexes = culledIndexes;
            }

            // Group all shared vertices together by position. We need to know adjacent faces
            // to do vertex normal remapping in the next step.
            QHash<QVector3D, QVector<quint32>> positionHash;
            for (quint32 i = 0; i < newIndexes.size(); ++i) {
                const quint32 index = newIndexes[i];
                const QVector3D position = vertexAttributes[index].aData.position;
                positionHash[position].append(i);
            }

            // Go through each vertex and calculate the normals by checking each
            // adjacent face that share the same vertex position, and create a smoothed
            // normal if the angle between thew face normals is less than the the
            // normalMergeAngle passed to this function (>= since this is cos(radian(angle)) )
            QVector<QPair<quint32, quint32>> remapIndexes;
            for (quint32 positionIndex = 0; positionIndex < newIndexes.size(); ++positionIndex) {
                const quint32 index = newIndexes[positionIndex];
                const QVector3D &position = vertexAttributes[index].aData.position;
                const QVector3D &faceNormal = faceNormals[positionIndex];
                QVector3D newNormal;
                // Find all vertices that share the same position
                const auto &sharedPositions = positionHash.value(position);
                for (auto positionIndex2 : sharedPositions) {
                    if (positionIndex == positionIndex2) {
                        // Don't test against the current face under test
                        newNormal += faceNormal;
                    } else {
                        const QVector3D &faceNormal2 = faceNormals[positionIndex2];
                        if (QVector3D::dotProduct(faceNormal2, faceNormal) >= normalMergeThreshold)
                            newNormal += faceNormal2;
                    }
                }

                // By normalizing here we get an averaged value of all smoothed normals
                QSSGUtils::vec3::normalize(newNormal);

                // Now that we know what the smoothed normal would be, check how differnt
                // that normal is from the normal that is already stored in the current
                // index. If the angle delta is greater than normalSplitAngle then we need
                // to create a new vertex entry (making a copy of the current one) and set
                // the new normal value, and reassign the current index to point to that new
                // vertex. Generally the LOD simplification process is such that the existing
                // normal will already be ideal until we start getting to the very low lod levels
                // which changes the topology in such a way that the original normal doesn't
                // make sense anymore, thus the need to provide a more reasonable value.
                const QVector3D &originalNormal = vertexAttributes[index].aData.normal;
                const float theta = QVector3D::dotProduct(originalNormal, newNormal);
                if (theta < normalSplitThreshold) {
                    splitVertexIndices.append(index);
                    splitVertexNormals.append(newNormal.normalized());
                    remapIndexes.append({positionIndex, splitVertexCount++});
                }
            }

            // Do index remap now that all new normals have been calculated
            for (auto pair : remapIndexes)
                newIndexes[pair.first] = pair.second;
        }

        lods.append({error * scaleFactor, newIndexes});
        indexTarget = qMax(newLength, indexTarget) * 2;
        lastIndexCount = newLength;

        if (error == 0.0f)
            break;
    }
    // Here we need to add the new index and vertex values from
    // splitVertexIndices and splitVertexNormals
    for (quint32 i = 0; i < splitVertexIndices.size(); ++i) {
        quint32 index = splitVertexIndices[i];
        QVector3D newNormal = splitVertexNormals[i];
        auto newVertex = vertexAttributes[index];
        newVertex.aData.normal = newNormal;
        vertexAttributes.append(newVertex);
    }

    return lods;
}

}

QSSGMesh::Mesh AssimpUtils::generateMeshData(const aiScene &scene,
                                             const MeshList &meshes,
                                             bool useFloatJointIndices,
                                             bool generateLevelsOfDetail,
                                             float normalMergeAngle,
                                             float normalSplitAngle,
                                             QString &errorString)
{
    Q_UNUSED(errorString);

    // All Mesh subsets are stored in the same Vertex Buffer so we need to make
    // sure that all attributes from each subset have common data by potentially
    // adding placeholder data or doing conversions as necessary.
    // So we need to walk through each subset first and see what the requirments are
    VertexDataRequirments requirments;
    requirments.useFloatJointIndices = useFloatJointIndices;
    for (const auto *mesh : meshes)
        requirments.collectRequirmentsForMesh(mesh);

    // This is the actual data we will pass to the QSSGMesh that will get filled by
    // each of the subset meshes
    QByteArray indexBufferData;
    VertexBufferDataExt vertexBufferData;
    QVector<SubsetEntryData> subsetData;

    // Since the vertex data of subsets are stored one after the other, the values in
    // the index buffer need to be augmented to reflect this offset. baseIndex is used
    // to track the new 0 value of a subset by keeping track of the current vertex
    // count as each new subset is added
    quint32 baseIndex = 0;

    // Always use 32-bit indices. Metal has a requirement of 4 byte alignment
    // for index buffer offsets, and we cannot risk hitting that.
    const QSSGMesh::Mesh::ComponentType indexType = QSSGMesh::Mesh::ComponentType::UnsignedInt32;

    for (const auto *mesh : meshes) {
        // Get the index values for just this mesh
        // The index values should be relative to this meshes
        // vertices and will later need to be corrected using
        // baseIndex to be relative to our combined vertex data
        QVector<quint32> indexes;
        indexes.reserve(mesh->mNumFaces * 3);
        for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
            const auto face = mesh->mFaces[faceIndex];
            // Faces should always have 3 indices
            Q_ASSERT(face.mNumIndices == 3);
            // Index data for now is relative to the local vertex locations
            // This must be corrected for later to be global
            indexes.append(quint32(face.mIndices[0]));
            indexes.append(quint32(face.mIndices[1]));
            indexes.append(quint32(face.mIndices[2]));
        }

        // Get the Vertex Attribute Data for this mesh
        auto vertexAttributes = getVertexAttributeData(mesh, requirments);

        // Starting point for index buffer offsets
        quint32 baseIndexOffset = indexBufferData.size() / QSSGMesh::MeshInternal::byteSizeForComponentType(indexType);
        QVector<quint32> lodIndexes;
        QVector<QSSGMesh::Mesh::Lod> meshLods;

        // Generate Automatic Mesh Levels of Detail
        if (generateLevelsOfDetail) {
            // Returns a list of lod pairs <distance, lodIndexList> sorted from smallest
            // to largest as this is how they are stored in the index buffer. We still need to
            // populate meshLods with push_front though because subset lod data is sorted from
            // highest detail to lowest
            auto lods = generateMeshLevelsOfDetail(vertexAttributes, indexes, normalMergeAngle, normalSplitAngle);
            for (const auto &lodPair : lods) {
                QSSGMesh::Mesh::Lod lod;
                lod.offset = baseIndexOffset;
                lod.count = lodPair.second.size();
                lod.distance = lodPair.first;
                meshLods.push_front(lod);
                baseIndexOffset += lod.count;
                // Optimize the vertex cache for this lod level
                auto currentLodIndexes = lodPair.second;
                QSSGMesh::optimizeVertexCache(currentLodIndexes.data(), currentLodIndexes.data(), currentLodIndexes.size(), vertexAttributes.size());
                lodIndexes += currentLodIndexes;
            }
        }

        // Write the results to the Global Index/Vertex/SubsetData buffers
        // Optimize the vertex chache for the original index values
        QSSGMesh::optimizeVertexCache(indexes.data(), indexes.data(), indexes.size(), vertexAttributes.size());

        // Write Index Buffer Data
        QVector<quint32> combinedIndexValues = lodIndexes + indexes;
        // Set the absolute index relative to the larger vertex buffer
        for (auto &index : combinedIndexValues)
            index += baseIndex;
        indexBufferData += QByteArray(reinterpret_cast<const char *>(combinedIndexValues.constData()),
                                      combinedIndexValues.size() * QSSGMesh::MeshInternal::byteSizeForComponentType(indexType));

        // Index Data is setup such that LOD indexes will come first
        // from lowest quality to original
        // | LOD3 | LOD2 | LOD1 | Original |
        // If there were no LOD levels then indexOffset just points to that here
        // baseIndexOffset has already been calculated to be correct at this point
        SubsetEntryData subsetEntry;
        subsetEntry.indexOffset = baseIndexOffset; // baseIndexOffset will be after lod indexes if available
        subsetEntry.indexLength = indexes.size(); // Yes, only original index values, because this is for the non-lod indexes
        subsetEntry.name = QString::fromUtf8(scene.mMaterials[mesh->mMaterialIndex]->GetName().C_Str());
        subsetEntry.lightmapWidth = 0;
        subsetEntry.lightmapHeight = 0;
        subsetEntry.lods = meshLods;
        subsetData.append(subsetEntry);

        // Fill the rest of the vertex data
        baseIndex += vertexAttributes.size(); // Final count of vertices added
        // Increase target buffers before adding data
        vertexBufferData.targetVData.resize(requirments.numMorphTargets);
        for (const auto &vertex : vertexAttributes)
            vertexBufferData.addVertexAttributeData(vertex, requirments);

    }

    // Now that we have all the data for the mesh, generate the entries list
    QVector<QSSGMesh::AssetVertexEntry> entries = vertexBufferData.createEntries(requirments);

    QVector<QSSGMesh::AssetMeshSubset> subsets;
    for (const SubsetEntryData &subset : subsetData) {
        subsets.append({
                           subset.name,
                           quint32(subset.indexLength),
                           quint32(subset.indexOffset),
                           0, // the builder will calculate the bounds from the position data
                           subset.lightmapWidth,
                           subset.lightmapHeight,
                           subset.lods
                       });
    }

    auto numTargetComponents = [](VertexDataRequirments req) {
        int num = 0;
        if (req.needsTargetPositionData)
            ++num;
        if (req.needsTargetNormalData)
            ++num;
        if (req.needsTargetTangentData)
            num += 2; // tangent and binormal
        if (req.needsTargetVertexColorData)
            ++num;
        if (req.needsTargetUV0Data)
            ++num;
        if (req.needsTargetUV1Data)
            ++num;
        return num;
    };

    QSSGMesh::Mesh mesh = QSSGMesh::Mesh::fromAssetData(entries,
                                                        indexBufferData,
                                                        indexType,
                                                        subsets,
                                                        requirments.numMorphTargets,
                                                        numTargetComponents(requirments));
    return mesh;
}

QT_END_NAMESPACE

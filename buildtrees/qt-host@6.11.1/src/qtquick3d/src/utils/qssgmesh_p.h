// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGMESHUTILITIES_P_H
#define QSSGMESHUTILITIES_P_H

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

#include <QtQuick3DUtils/private/qtquick3dutilsglobal_p.h>

#include <QtQuick3DUtils/private/qssgbounds3_p.h>

#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

#include <QtCore/qstring.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

struct QSSGRenderVertexBufferEntry
{
    QByteArray m_name;
    /** Datatype of the this entry points to in the buffer */
    QSSGRenderComponentType m_componentType;
    /** Number of components of each data member. 1,2,3, or 4.  Don't be stupid.*/
    quint32 m_numComponents;
    /** Offset from the beginning of the buffer of the first item */
    quint32 m_firstItemOffset;

    QSSGRenderVertexBufferEntry(const QByteArray &nm,
                                QSSGRenderComponentType type,
                                quint32 numComponents,
                                quint32 firstItemOffset = 0)
        : m_name(nm), m_componentType(type), m_numComponents(numComponents), m_firstItemOffset(firstItemOffset)
    {
    }

    QSSGRenderVertexBufferEntry()
        : m_componentType(QSSGRenderComponentType::Float32), m_numComponents(0), m_firstItemOffset(0)
    {
    }

    QSSGRenderVertexBufferEntry(const QSSGRenderVertexBufferEntry &inOther)
        : m_name(inOther.m_name)
        , m_componentType(inOther.m_componentType)
        , m_numComponents(inOther.m_numComponents)
        , m_firstItemOffset(inOther.m_firstItemOffset)
    {
    }

    QSSGRenderVertexBufferEntry &operator=(const QSSGRenderVertexBufferEntry &inOther)
    {
        if (this != &inOther) {
            m_name = inOther.m_name;
            m_componentType = inOther.m_componentType;
            m_numComponents = inOther.m_numComponents;
            m_firstItemOffset = inOther.m_firstItemOffset;
        }
        return *this;
    }
};

namespace QSSGMesh {

struct AssetVertexEntry;
struct AssetMeshSubset;
struct RuntimeMeshData;
struct AssetLodEntry;

class Q_QUICK3DUTILS_EXPORT Mesh
{
public:
    using DrawMode = QSSGRenderDrawMode;
    using Winding = QSSGRenderWinding;
    using ComponentType = QSSGRenderComponentType;

    struct VertexBufferEntry {
        ComponentType componentType = ComponentType::Float32;
        quint32 componentCount = 0;
        quint32 offset = 0;
        QByteArray name;

        QSSGRenderVertexBufferEntry toRenderVertexBufferEntry() const {
            return QSSGRenderVertexBufferEntry(name,
                                               QSSGRenderComponentType(componentType),
                                               componentCount,
                                               offset);
        }
    };

    struct VertexBuffer {
        quint32 stride = 0;
        QVector<VertexBufferEntry> entries;
        QByteArray data;
    };

    struct IndexBuffer {
        ComponentType componentType = ComponentType::UnsignedInt32;
        QByteArray data;
    };

    struct TargetBuffer {
        quint32 numTargets = 0;
        QVector<VertexBufferEntry> entries;
        QByteArray data;
    };

    struct SubsetBounds {
        QVector3D min;
        QVector3D max;
    };

    struct Lod {
        quint32 count = 0;
        quint32 offset = 0;
        float distance = 0.0f;
    };

    struct Subset {
        QString name;
        SubsetBounds bounds;
        quint32 count = 0;
        quint32 offset = 0;
        QSize lightmapSizeHint;
        QVector<Lod> lods;
    };

    // can just return by value (big data is all implicitly shared)
    VertexBuffer vertexBuffer() const { return m_vertexBuffer; }
    IndexBuffer indexBuffer() const { return m_indexBuffer; }
    TargetBuffer targetBuffer() const { return m_targetBuffer; }
    QVector<Subset> subsets() const { return m_subsets; }

    // id 0 == first, otherwise has to match
    static Mesh loadMesh(QIODevice *device, quint32 id = 0);

    static QMap<quint32, Mesh> loadAll(QIODevice *device);

    static Mesh fromAssetData(const QVector<AssetVertexEntry> &vbufEntries,
                              const QByteArray &indexBufferData,
                              ComponentType indexComponentType,
                              const QVector<AssetMeshSubset> &subsets,
                              quint32 numTargets = 0, quint32 numTargetComps = 0);

    static Mesh fromRuntimeData(const RuntimeMeshData &data,
                                QString *error);

    bool isValid() const { return !m_subsets.isEmpty(); }

    DrawMode drawMode() const { return m_drawMode; }
    Winding winding() const { return m_winding; }

    // id 0 == generate new id; otherwise uses it as-is, and must be an unused one
    quint32 save(QIODevice *device, quint32 id = 0) const;

    bool hasLightmapUVChannel() const;
    bool createLightmapUVChannel(float texelsPerUnit, const QMatrix4x4 &scale);

private:
    DrawMode m_drawMode = DrawMode::Triangles;
    Winding m_winding = Winding::CounterClockwise;
    VertexBuffer m_vertexBuffer;
    IndexBuffer m_indexBuffer;
    TargetBuffer m_targetBuffer;
    QVector<Subset> m_subsets;
    friend struct MeshInternal;
};

struct Q_QUICK3DUTILS_EXPORT AssetVertexEntry // for asset importer plugins (Assimp, FBX)
{
    QByteArray name;
    QByteArray data;
    Mesh::ComponentType componentType = Mesh::ComponentType::Float32;
    quint32 componentCount = 0;
    qint32 morphTargetId = -1; // -1 menas that this entry belongs to the original mesh.
};

struct Q_QUICK3DUTILS_EXPORT AssetMeshSubset // for asset importer plugins (Assimp, FBX)
{
    QString name;
    quint32 count = 0;
    quint32 offset = 0;
    quint32 boundsPositionEntryIndex = std::numeric_limits<quint32>::max();
    quint32 lightmapWidth = 0;
    quint32 lightmapHeight = 0;
    QVector<Mesh::Lod> lods;
};

struct Q_QUICK3DUTILS_EXPORT RuntimeMeshData // for custom geometry (QQuick3DGeometry, QSSGRenderGeometry)
{
    struct Attribute {
        enum Semantic {
            IndexSemantic = 0,
            PositionSemantic,                       // attr_pos
            NormalSemantic,                         // attr_norm
            TexCoordSemantic,                       // attr_uv0
            TangentSemantic,                        // attr_textan
            BinormalSemantic,                       // attr_binormal
            JointSemantic,                          // attr_joints
            WeightSemantic,                         // attr_weights
            ColorSemantic,                          // attr_color
            TexCoord1Semantic,                      // attr_uv1
            TexCoord0Semantic = TexCoordSemantic    // attr_uv0
        };

        Semantic semantic = PositionSemantic;
        Mesh::ComponentType componentType = Mesh::ComponentType::Float32;
        int offset = 0;

        int componentCount() const {
            switch (semantic) {
            case IndexSemantic:             return 1;
            case PositionSemantic:          return 3;
            case NormalSemantic:            return 3;
            case TexCoord0Semantic:         return 2;
            case TexCoord1Semantic:         return 2;
            case TangentSemantic:           return 3;
            case BinormalSemantic:          return 3;
            case JointSemantic:             return 4;
            case WeightSemantic:            return 4;
            case ColorSemantic:             return 4;
            }
            Q_UNREACHABLE_RETURN(0);
        }
    };

    struct TargetAttribute {
        Attribute attr;
        int targetId;
        int stride;
    };

    static const int MAX_ATTRIBUTES = 16;
    static const int MAX_TARGET_ATTRIBUTES = 32;

    void clear()
    {
        clearVertexAndIndex();
        clearTarget();
    }
    void clearVertexAndIndex()
    {
        m_vertexBuffer.clear();
        m_indexBuffer.clear();
        m_subsets.clear();
        m_attributeCount = 0;
        m_primitiveType = Mesh::DrawMode::Triangles;
    }
    void clearTarget()
    {
        m_targetBuffer.clear();
        m_targetAttributeCount = 0;
    }

    QByteArray m_vertexBuffer;
    QByteArray m_indexBuffer;
    QByteArray m_targetBuffer;
    QVector<Mesh::Subset> m_subsets;

    Attribute m_attributes[MAX_ATTRIBUTES];
    int m_attributeCount = 0;
    TargetAttribute m_targetAttributes[MAX_TARGET_ATTRIBUTES];
    int m_targetAttributeCount = 0;
    Mesh::DrawMode m_primitiveType = Mesh::DrawMode::Triangles;
    int m_stride = 0;
};

struct Q_QUICK3DUTILS_EXPORT MeshInternal
{
    struct MultiMeshInfo {
        quint32 fileId = 0;
        quint32 fileVersion = 0;
        QMap<quint32, quint64> meshEntries;
        static const quint32 FILE_ID = 555777497;
        static const quint32 FILE_VERSION = 1;
        bool isValid() const {
            return fileId == FILE_ID && fileVersion == FILE_VERSION;
        }
        static MultiMeshInfo withDefaults() {
            return { FILE_ID, FILE_VERSION, {} };
        }
    };

    struct MeshDataHeader {
        quint32 fileId = 0;
        quint16 fileVersion = 0;
        quint16 flags = 0;
        quint32 sizeInBytes = 0;

        static const quint32 FILE_ID = 3365961549;

        // Version 3 and 4 is only different in the "offset" values that are
        // all zeroes in version 4 because they are not used by the new mesh
        // reader. So to support both with the new loader, no branching is
        // needed at all, it just needs to accept both versions.
        static const quint32 LEGACY_MESH_FILE_VERSION = 3;
        // Version 5 differs from 4 with the added lightmapSizeHint per subset.
        // This needs branching in the deserializer.
        // Version 6 differs from 5 with additional lodCount per subset as well
        // as a list of Level of Detail data after the subset names.
        // Version 7 will split the morph target data
        static const quint32 FILE_VERSION = 7;

        static MeshDataHeader withDefaults() {
            return { FILE_ID, FILE_VERSION, 0, 0 };
        }

        bool isValid() const {
            return fileId == FILE_ID
                    && fileVersion <= FILE_VERSION
                    && fileVersion >= LEGACY_MESH_FILE_VERSION;
        }

        bool hasLightmapSizeHint() const {
            return fileVersion >= 5;
        }

        bool hasLodDataHint() const {
            return fileVersion >= 6;
        }

        bool hasSeparateTargetBuffer() const {
            return fileVersion >= 7;
        }
    };

    struct MeshOffsetTracker {
        quint32 startOffset = 0;
        quint32 byteCounter = 0;

        MeshOffsetTracker(int offset)
            : startOffset(offset) {}

        int offset() {
            return startOffset + byteCounter;
        }

        quint32 alignedAdvance(int advanceAmount) {
            advance(advanceAmount);
            quint32 alignmentAmount = 4 - (byteCounter % 4);
            byteCounter += alignmentAmount;
            return alignmentAmount;
        }

        void advance(int advanceAmount) {
            byteCounter += advanceAmount;
        }
    };

    struct Subset {
        QByteArray rawNameUtf16;
        quint32 nameLength = 0;
        Mesh::SubsetBounds bounds;
        quint32 offset = 0;
        quint32 count = 0;
        QSize lightmapSizeHint;
        quint32 lodCount = 0;

        Mesh::Subset toMeshSubset() const {
            Mesh::Subset subset;
            if (nameLength > 0)
                subset.name = QString::fromUtf16(reinterpret_cast<const char16_t *>(rawNameUtf16.constData()), nameLength - 1);
            subset.bounds.min = bounds.min;
            subset.bounds.max = bounds.max;
            subset.count = count;
            subset.offset = offset;
            subset.lightmapSizeHint = lightmapSizeHint;
            subset.lods.resize(lodCount);
            return subset;
        }
    };

    static MultiMeshInfo readFileHeader(QIODevice *device);
    static void writeFileHeader(QIODevice *device, const MultiMeshInfo &meshFileInfo);
    static quint64 readMeshData(QIODevice *device, quint64 offset, Mesh *mesh, MeshDataHeader *header);
    static void writeMeshHeader(QIODevice *device, const MeshDataHeader &header);
    static quint64 writeMeshData(QIODevice *device, const Mesh &mesh);

    static quint32 byteSizeForComponentType(Mesh::ComponentType componentType) { return quint32(QSSGBaseTypeHelpers::getSizeOfType(componentType)); }

    static const char *getPositionAttrName() { return "attr_pos"; }
    static const char *getNormalAttrName() { return "attr_norm"; }
    static const char *getUV0AttrName() { return "attr_uv0"; }
    static const char *getUV1AttrName() { return "attr_uv1"; }
    static const char *getLightmapUVAttrName() { return "attr_lightmapuv"; }
    static const char *getTexTanAttrName() { return "attr_textan"; }
    static const char *getTexBinormalAttrName() { return "attr_binormal"; }
    static const char *getColorAttrName() { return "attr_color"; }
    static const char *getJointAttrName() { return "attr_joints"; }
    static const char *getWeightAttrName() { return "attr_weights"; }

    static QSSGBounds3 calculateSubsetBounds(const Mesh::VertexBufferEntry &entry,
                                             const QByteArray &vertexBufferData,
                                             quint32 vertexBufferStride,
                                             const QByteArray &indexBufferData,
                                             Mesh::ComponentType indexComponentType,
                                             quint32 subsetCount,
                                             quint32 subsetOffset);
};

size_t Q_QUICK3DUTILS_EXPORT simplifyMesh(unsigned int* destination,
                                          const unsigned int* indices,
                                          size_t indexCount,
                                          const float* vertexPositions,
                                          size_t vertexCount,
                                          size_t vertexPositionsStride,
                                          size_t targetIndexCount,
                                          float targetError,
                                          unsigned int options,
                                          float* resultError);

float Q_QUICK3DUTILS_EXPORT simplifyScale(const float* vertexPositions,
                                          size_t vertexCount,
                                          size_t vertexPositionsStride);

void Q_QUICK3DUTILS_EXPORT optimizeVertexCache(unsigned int* destination,
                                               const unsigned int* indices,
                                               size_t indexCount,
                                               size_t vertexCount);

} // namespace QSSGMesh

QT_END_NAMESPACE

#endif // QSSGMESHUTILITIES_P_H

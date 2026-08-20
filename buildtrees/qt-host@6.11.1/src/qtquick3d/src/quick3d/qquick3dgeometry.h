// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef Q_QUICK3D_GEOMETRY_H
#define Q_QUICK3D_GEOMETRY_H

#include <QtQuick3D/qquick3dobject.h>

QT_BEGIN_NAMESPACE

class QQuick3DGeometryPrivate;

class Q_QUICK3D_EXPORT QQuick3DGeometry : public QQuick3DObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuick3DGeometry)

    QML_NAMED_ELEMENT(Geometry)
    QML_UNCREATABLE("Geometry is Abstract")
public:
    explicit QQuick3DGeometry(QQuick3DObject *parent = nullptr);
    ~QQuick3DGeometry() override;

    enum class PrimitiveType {
        Points,
        LineStrip,
        Lines,
        TriangleStrip,
        TriangleFan,
        Triangles
    };

    struct Attribute {
        enum Semantic {
            IndexSemantic,
            PositionSemantic,                     // attr_pos
            NormalSemantic,                       // attr_norm
            TexCoordSemantic,                     // attr_uv0
            TangentSemantic,                      // attr_textan
            BinormalSemantic,                     // attr_binormal
            JointSemantic,                        // attr_joints
            WeightSemantic,                       // attr_weights
            ColorSemantic,                        // attr_color
            TargetPositionSemantic,               // attr_tpos0
            TargetNormalSemantic,                 // attr_tnorm0
            TargetTangentSemantic,                // attr_ttan0
            TargetBinormalSemantic,               // attr_tbinorm0
            TexCoord1Semantic,                    // attr_uv1
            TexCoord0Semantic = TexCoordSemantic  // for compatibility
        };
        enum ComponentType {
            U16Type,
            U32Type,
            I32Type,
            F32Type
        };
        Semantic semantic = PositionSemantic;
        int offset = -1;
        ComponentType componentType = F32Type;
    };

    struct TargetAttribute {
        quint32 targetId = 0;
        Attribute attr;
        int stride = 0;
    };

    QByteArray vertexData() const;
    QByteArray indexData() const;
    int attributeCount() const;
    Attribute attribute(int index) const;
    PrimitiveType primitiveType() const;
    QVector3D boundsMin() const;
    QVector3D boundsMax() const;
    int stride() const;

    void setVertexData(const QByteArray &data);
    void setVertexData(int offset, const QByteArray &data);
    void setIndexData(const QByteArray &data);
    void setIndexData(int offset, const QByteArray &data);
    void setStride(int stride);
    void setBounds(const QVector3D &min, const QVector3D &max);
    void setPrimitiveType(PrimitiveType type);

    void addAttribute(Attribute::Semantic semantic, int offset,
                      Attribute::ComponentType componentType);
    void addAttribute(const Attribute &att);

    Q_REVISION(6, 3) int subsetCount() const;
    Q_REVISION(6, 3) QVector3D subsetBoundsMin(int subset) const;
    Q_REVISION(6, 3) QVector3D subsetBoundsMax(int subset) const;
    Q_REVISION(6, 3) int subsetOffset(int subset) const;
    Q_REVISION(6, 3) int subsetCount(int subset) const;
    Q_REVISION(6, 3) QString subsetName(int subset) const;
    Q_REVISION(6, 3) void addSubset(int offset, int count, const QVector3D &boundsMin, const QVector3D &boundsMax, const QString &name = {});

    Q_REVISION(6, 6) QByteArray targetData() const;
    Q_REVISION(6, 6) void setTargetData(const QByteArray &data);
    Q_REVISION(6, 6) void setTargetData(int offset, const QByteArray &data);
    Q_REVISION(6, 6) TargetAttribute targetAttribute(int index) const;
    Q_REVISION(6, 6) int targetAttributeCount() const;
    Q_REVISION(6, 6) void addTargetAttribute(quint32 targetId,
                                             Attribute::Semantic semantic, int offset,
                                             int stride = 0);
    Q_REVISION(6, 6) void addTargetAttribute(const TargetAttribute &att);

    void clear();

Q_SIGNALS:
    void geometryNodeDirty();
    Q_REVISION(6, 7) void geometryChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;
};

QT_END_NAMESPACE

#endif // Q_QUICK3D_GEOMETRY_H

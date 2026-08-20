// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qquick3dgeometry.h>
#include <qvector3d.h>

class TestGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(bool indexed READ indexed WRITE setIndexed NOTIFY indexedChanged)
    QML_NAMED_ELEMENT(TestGeometry)
public:
    TestGeometry()
    {
        updateData();
    }

    bool indexed() const
    {
        return m_indexed;
    }

public Q_SLOTS:
    void setIndexed(bool indexed)
    {
        if (m_indexed == indexed) return;
        m_indexed = indexed;
        updateData();
    }
Q_SIGNALS:
    void indexedChanged();

private:
    struct Vertex
    {
        QVector3D pos;
        QVector3D norm;
        QVector2D texcoord;
    };
    const int primitiveCount = 200;
    inline QVector3D minimum(const QVector3D &v1, const QVector3D &v2) Q_DECL_NOTHROW { return { qMin(v1.x(), v2.x()), qMin(v1.y(), v2.y()), qMin(v1.z(), v2.z()) }; }
    inline QVector3D maximum(const QVector3D &v1, const QVector3D &v2) Q_DECL_NOTHROW { return { qMax(v1.x(), v2.x()), qMax(v1.y(), v2.y()), qMax(v1.z(), v2.z()) }; }
    void updateData()
    {
        clear();
        QVector3D min = QVector3D(1, 1, 1) * 100000;
        QVector3D max = QVector3D(1, 1, 1) * -100000;
        QByteArray vertexData;
        QByteArray indexData;

        QVector3D off = QVector3D(sqrt(primitiveCount) - 10, sqrt(primitiveCount) - 10, 0) * -0.5f;

        for (int i = 0; i < primitiveCount / 2; i++) {
            int u = i / 10;
            int v = i % 10;
            Vertex v0 = {QVector3D(u, v, 0) + off, QVector3D(0, 0, 1), QVector2D(u, v) / primitiveCount};
            Vertex v1 = {QVector3D(u, v+1, 0) + off, QVector3D(0, 0, 1), QVector2D(u+1, v) / primitiveCount};
            Vertex v2 = {QVector3D(u+1, v+1, 0) + off, QVector3D(0, 0, 1), QVector2D(u, v+1) / primitiveCount};
            Vertex v3 = {QVector3D(u+1, v+1, 0) + off, QVector3D(0, 0, 1), QVector2D(u+1, v+1) / primitiveCount};
            Vertex v4 = {QVector3D(u+1, v, 0) + off, QVector3D(0, 0, 1), QVector2D(u, v+1) / primitiveCount};
            Vertex v5 = {QVector3D(u, v, 0) + off, QVector3D(0, 0, 1), QVector2D(u+1, v) / primitiveCount};
            vertexData.append(reinterpret_cast<const char *>(&v0), sizeof(Vertex));
            vertexData.append(reinterpret_cast<const char *>(&v1), sizeof(Vertex));
            vertexData.append(reinterpret_cast<const char *>(&v2), sizeof(Vertex));
            vertexData.append(reinterpret_cast<const char *>(&v3), sizeof(Vertex));
            vertexData.append(reinterpret_cast<const char *>(&v4), sizeof(Vertex));
            vertexData.append(reinterpret_cast<const char *>(&v5), sizeof(Vertex));
            if (m_indexed) {
                quint32 i0 = 6*i;
                quint32 i1 = 6*i+1;
                quint32 i2 = 6*i+2;
                quint32 i3 = 6*i+3;
                quint32 i4 = 6*i+4;
                quint32 i5 = 6*i+5;
                indexData.append(reinterpret_cast<const char *>(&i0), 4);
                indexData.append(reinterpret_cast<const char *>(&i1), 4);
                indexData.append(reinterpret_cast<const char *>(&i2), 4);
                indexData.append(reinterpret_cast<const char *>(&i3), 4);
                indexData.append(reinterpret_cast<const char *>(&i4), 4);
                indexData.append(reinterpret_cast<const char *>(&i5), 4);
            }
            min = minimum(min, v0.pos);
            min = minimum(min, v1.pos);
            min = minimum(min, v2.pos);
            max = maximum(max, v0.pos);
            max = maximum(max, v1.pos);
            max = maximum(max, v2.pos);
            min = minimum(min, v3.pos);
            min = minimum(min, v4.pos);
            min = minimum(min, v5.pos);
            max = maximum(max, v3.pos);
            max = maximum(max, v4.pos);
            max = maximum(max, v5.pos);
        }

        addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                     QQuick3DGeometry::Attribute::ComponentType::F32Type);
        addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, sizeof(QVector3D),
                     QQuick3DGeometry::Attribute::ComponentType::F32Type);
        addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic, 2 * sizeof(QVector3D),
                     QQuick3DGeometry::Attribute::ComponentType::F32Type);
        if (m_indexed) {
            addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                         QQuick3DGeometry::Attribute::ComponentType::U32Type);
            setIndexData(indexData);
        }
        setStride(sizeof(Vertex));
        setVertexData(vertexData);

        setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
        setBounds(min, max);
    }
    bool m_indexed = false;
};

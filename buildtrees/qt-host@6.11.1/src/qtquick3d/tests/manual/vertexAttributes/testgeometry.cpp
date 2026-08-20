// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testgeometry.h"

#include <qvector4d.h>
#include <qvector3d.h>
#include <qvector2d.h>
#include <qmath.h>
#include <qcolor.h>

struct Vertex {
    QVector3D position;
    QVector3D normal;
    QVector3D tangent;
    QVector3D binormal;
    QVector2D uv0;
    QVector2D uv1;
    QVector4D color;

    float pad[4];
};

static const int s_vertexSize = sizeof(Vertex);
Q_STATIC_ASSERT(s_vertexSize == 96);

template <typename T>
void byteArrayPtr(T *&ptr, QByteArray &positions)
{
    ptr = reinterpret_cast<T *>(positions.data());
}
template <typename T>
void byteArrayPtr(const T *&ptr, const QByteArray &positions)
{
    ptr = reinterpret_cast<const T *>(positions.constData());
}

struct Face {
    quint32 v0, v1, v2;
    Face() {}
    Face(quint32 a, quint32 b, quint32 c)
        : v0(a), v1(b), v2(c) {}
    void ensureHandedness(const QByteArray &pos, QVector3D n)
    {
        const QVector3D *p = nullptr;
        byteArrayPtr(p, pos);
        QVector3D a = p[v1] - p[v0];
        QVector3D b = p[v2] - p[v1];
        QVector3D c = QVector3D::crossProduct(a, b);
        if (QVector3D::dotProduct(c, n) < 0.0f)
            qSwap(v1, v2);
    }
};

static void packVertices(const QByteArray &pos, const QByteArray &norm,
                         const QByteArray &uv0, const QByteArray &uv1,
                         const QByteArray &tan, const QByteArray &bin,
                         const QByteArray &col, QByteArray &dest)
{
    int count = int(pos.size() / sizeof(QVector3D));
    QByteArray vertices;
    vertices.resize(count * s_vertexSize);
    vertices.fill(0);

    const QVector3D *p = reinterpret_cast<const QVector3D *>(pos.constData());
    const QVector3D *n = norm.size() ? reinterpret_cast<const QVector3D *>(norm.constData()) : nullptr;
    const QVector2D *u0 = uv0.size() ? reinterpret_cast<const QVector2D *>(uv0.constData()) : nullptr;
    const QVector2D *u1 = uv1.size() ? reinterpret_cast<const QVector2D *>(uv1.constData()) : nullptr;
    const QVector3D *t = tan.size() ? reinterpret_cast<const QVector3D *>(tan.constData()) : nullptr;
    const QVector3D *bn = bin.size() ? reinterpret_cast<const QVector3D *>(bin.constData()) : nullptr;
    const QVector4D *c = col.size() ? reinterpret_cast<const QVector4D *>(col.constData()) : nullptr;

    Vertex *vert = reinterpret_cast<Vertex *>(vertices.data());
    for (int i = 0; i < count; i++) {
        Vertex &v = vert[i];
        v.position = p[i];
        if (n)
            v.normal = n[i];
        if (u0)
            v.uv0 = u0[i];
        if (u1)
            v.uv1 = u1[i];
        if (t)
            v.tangent = t[i];
        if (bn)
            v.binormal = bn[i];
        if (c)
            v.color = c[i];
    }
    dest = vertices;
}

static QByteArray s_normals = QByteArray();

QByteArray &generateNormals(const QByteArray &positions, const QByteArray &indices)
{
    if (!s_normals.size()) {
        int count = int(indices.size() / sizeof(Face));
        s_normals.resize(positions.size());
        s_normals.fill(0);
        const Face *faces = reinterpret_cast<const Face *>(indices.constData());
        QVector3D *n = reinterpret_cast<QVector3D *>(s_normals.data());
        const QVector3D *p = reinterpret_cast<const QVector3D *>(positions.constData());
        for (int i = 0; i < count; i++) {
            const Face &f = faces[i];
            QVector3D u = p[f.v1] - p[f.v0];
            QVector3D v = p[f.v2] - p[f.v0];
            QVector3D normal = u.crossProduct(u, v).normalized();
            if (normal.y() < 0.0f)
                normal *= -1.0f;
            n[f.v0] += normal;
            n[f.v1] += normal;
            n[f.v2] += normal;
        }
        count = int(positions.size() / sizeof(QVector3D));
        for (int i = 0; i < count; i++)
            n[i] = n[i].normalized();
    }
    return s_normals;
}

static QByteArray s_tangents = QByteArray();

QByteArray &generateTangents(const QByteArray &positions, const QByteArray &normals, const QByteArray &indices)
{
    if (!s_tangents.size()) {
        int count = int(indices.size() / sizeof(Face));
        s_tangents.resize(positions.size());
        s_tangents.fill(0);
        const Face *faces = reinterpret_cast<const Face *>(indices.constData());
        const QVector3D *n = reinterpret_cast<const QVector3D *>(normals.constData());
        QVector3D *t = reinterpret_cast<QVector3D *>(s_tangents.data());
        const QVector3D *p = reinterpret_cast<const QVector3D *>(positions.constData());
        for (int i = 0; i < count; i++) {
            const Face &f = faces[i];
            QVector3D u = p[f.v1] - p[f.v0];
            t[f.v0] += QVector3D::crossProduct(n[f.v0], u).normalized();
            t[f.v1] += QVector3D::crossProduct(n[f.v1], u).normalized();
            t[f.v2] += QVector3D::crossProduct(n[f.v2], u).normalized();
        }
        count = int(positions.size() / sizeof(QVector3D));
        for (int i = 0; i < count; i++)
            t[i] = t[i].normalized();
    }
    return s_tangents;
}

static QByteArray s_binormals = QByteArray();

QByteArray &generateBinormals(const QByteArray &normals, const QByteArray &tangents)
{
    if (!s_binormals.size()) {
        int count = int(normals.size() / sizeof(QVector3D));
        s_binormals.resize(normals.size());
        s_binormals.fill(0);

        const QVector3D *n = reinterpret_cast<const QVector3D *>(normals.constData());
        const QVector3D *t = reinterpret_cast<const QVector3D *>(tangents.constData());
        QVector3D *bn = reinterpret_cast<QVector3D *>(s_binormals.data());

        for (int i = 0; i < count; i++)
            bn[i] = QVector3D::crossProduct(n[i], t[i]).normalized();
    }
    return s_binormals;
}

static QByteArray s_texcoords0 = QByteArray();

QByteArray &generateTexcoords0(const QByteArray &positions, const QByteArray &normals,
                     QVector3D &minPos, QVector3D &maxPos, float tstep)
{
    if (!s_texcoords0.size()) {
        int count = int(positions.size() / sizeof(QVector3D));
        s_texcoords0.resize(count * sizeof(QVector2D));
        s_texcoords0.fill(0);
        QVector2D *t = reinterpret_cast<QVector2D *>(s_texcoords0.data());
        const QVector3D *n = normals.size() ? reinterpret_cast<const QVector3D *>(normals.constData()) : nullptr;
        const QVector3D *p = reinterpret_cast<const QVector3D *>(positions.constData());

        float xScale = 1.f / (maxPos.x() - minPos.x());
        float zScale = 1.f / (maxPos.z() - minPos.z());

        for (int i = 0; i < count; ++i) {
            float u = p[i].x() * xScale;
            float v = p[i].z() * zScale;
            if (n) {
                const QVector3D &normal = n[i];
                u -= tstep * normal.x() * xScale;
                v -= tstep * normal.z() * zScale;
            }
            t[i] = QVector2D(u, v);
        }
    }
    return s_texcoords0;
}

static QByteArray s_texcoords1 = QByteArray();

QByteArray &generateTexcoords1(const QByteArray &positions)
{
    if (!s_texcoords1.size()) {
        int count = int(positions.size() / sizeof(QVector3D));
        s_texcoords1.resize(count * sizeof(QVector2D));
        s_texcoords1.fill(0);
        QVector2D *t = reinterpret_cast<QVector2D *>(s_texcoords1.data());
        const QVector3D *p = reinterpret_cast<const QVector3D *>(positions.constData());

        for (int i = 0; i < count; ++i) {
            float u = 1.0 + p->y();
            float v = 1.0 - p->y();
            t[i] = QVector2D(u, v);
        }
    }
    return s_texcoords1;
}

static QByteArray s_colors = QByteArray();

QByteArray &generateColors(const QByteArray &positions)
{
    if (!s_colors.size()) {
        int count = int(positions.size() / sizeof(QVector3D));
        s_colors.resize(count * sizeof(QVector4D));
        s_colors.fill(0);
        QVector4D colorF = QVector4D(1.f, 0.5f, 0.5f, 1.0f);
        QVector4D colorL = QVector4D(0.125f, 0.125f, 0.125f, 1.0f);
        QVector4D *c = reinterpret_cast<QVector4D *>(s_colors.data());
        const QVector3D *p = reinterpret_cast<const QVector3D *>(positions.constData());
        float div = 5.0f;
        float w = 1.0f;
        for (int i = 0; i < count; i++) {
            float y = p[i].y();
            float t = qFloor(y / div);
            y = y - t * div;
            if (qAbs(y) < w)
                c[i] = colorL;
            else
                c[i] = colorF;
        }
    }
    return s_colors;
}

template <typename T>
quint32 insertData(const T &c, QByteArray &data)
{
    int count = int(data.size() / sizeof(T));
    QByteArray d;
    d.resize(sizeof(T));
    memcpy(d.data(), &c, sizeof(T));
    data.append(d);
    return count;
}

quint32 findOrInsertVertex(const QVector3D v, QByteArray &vertices, int &lsty, int step)
{
    const QVector3D *ptr = reinterpret_cast<const QVector3D *>(vertices.constData());
    int count = int(vertices.size() / sizeof(QVector3D));
    for (int i = 0; i < count; ++i) {
        if (qFuzzyCompare(v, ptr[(i + lsty) % count])) {
            lsty = (i + lsty) % count;
            return lsty;
        }
    }
    lsty = qMax(count - step, 0);
    return insertData(v, vertices);
}

float camp(int x, int z, float fx, float fz, float amp)
{
    float y = cos(qDegreesToRadians(z * fz)) * amp;
    y += sin(qDegreesToRadians(x * fx)) * amp;
    y *= 0.5f;
    return y;
}

static QByteArray s_positions = QByteArray();
static QByteArray s_indices = QByteArray();

static QByteArray generatePositions(int count, QByteArray &outIndices, bool flat, QVector3D &minPos, QVector3D &maxPos, float size)
{
    float xfrequency = 3*360 / (100 / (size * count) * count);
    float zfrequency = 7*360 / (100 / (size * count) * count);
    float aplitude = 10.f;
    float aplitude2 = 5.f;
    float xfrequency2 = 360 / (100 / (size * count) * count);
    float zfrequency2 = 360 / (100 / (size * count) * count);

    minPos = QVector3D(0, -aplitude2 - aplitude, 0);
    maxPos = QVector3D(count * size, aplitude2 + aplitude, count * size);
    if (!s_positions.size()) {
        int lsdy = 0;
        for (int z = 0; z < count; z++) {
            for (int x = 0; x < count; x++) {
                float y0 = camp(x, z, xfrequency, zfrequency, aplitude);
                float y1 = camp(x, z+1, xfrequency, zfrequency, aplitude);
                float y2 = camp(x+1, z+1, xfrequency, zfrequency, aplitude);
                float y3 = camp(x+1, z, xfrequency, zfrequency, aplitude);

                y0 += camp(x, z, xfrequency2, zfrequency2, aplitude2);
                y1 += camp(x, z+1, xfrequency2, zfrequency2, aplitude2);
                y2 += camp(x+1, z+1, xfrequency2, zfrequency2, aplitude2);
                y3 += camp(x+1, z, xfrequency2, zfrequency2, aplitude2);

                float x0 = x * size;
                float x1 = (x+1) * size;
                float z0 = z * size;
                float z1 = (z+1) * size;
                QVector3D v0(x0, y0, z0);
                QVector3D v1(x0, y1, z1);
                QVector3D v2(x1, y2, z1);
                QVector3D v3(x1, y3, z0);
                quint32 t0;
                quint32 t1;
                quint32 t2;
                quint32 t3;
                if (!flat) {
                    t0 = findOrInsertVertex(v0, s_positions, lsdy, count);
                    t1 = findOrInsertVertex(v1, s_positions, lsdy, count);
                    t2 = findOrInsertVertex(v2, s_positions, lsdy, count);
                    t3 = findOrInsertVertex(v3, s_positions, lsdy, count);
                } else {
                    t0 = insertData(v0, s_positions);
                    t1 = insertData(v1, s_positions);
                    t2 = insertData(v2, s_positions);
                    t3 = insertData(v3, s_positions);
                }
                QVector3D n = QVector3D::crossProduct(v1 - v0, v2 - v1).normalized();
                if (n.y() < 0.0f)
                    n *= -1.0f;
                Face f0(t0, t1, t2);
                Face f1(t0, t2, t3);
                f0.ensureHandedness(s_positions, n);
                f1.ensureHandedness(s_positions, n);
                insertData(f0, s_indices);
                insertData(f1, s_indices);
            }
        }
    }
    outIndices = s_indices;
    return s_positions;
}

TestGeometry::TestGeometry(QQuick3DObject *parent)
    : QQuick3DGeometry(parent)
{

}

bool TestGeometry::position() const
{
    return m_position;
}

bool TestGeometry::normal() const
{
    return m_normal;
}

bool TestGeometry::texcoord0() const
{
    return m_texcoord0;
}

bool TestGeometry::texcoord1() const
{
    return m_texcoord1;
}

bool TestGeometry::tangent() const
{
    return m_tangent;
}

bool TestGeometry::binormal() const
{
    return m_binormal;
}

bool TestGeometry::color() const
{
    return m_color;
}

void TestGeometry::setPosition(bool enable)
{
    if (enable == m_position)
        return;
    m_position = enable;
    emit positionChanged();
    m_dirty = true;
}

void TestGeometry::setNormal(bool enable)
{
    if (enable == m_normal)
        return;
    m_normal = enable;
    emit normalChanged();
    m_dirty = true;
}

void TestGeometry::setTexcoord0(bool enable)
{
    if (enable == m_texcoord0)
        return;
    m_texcoord0 = enable;
    emit texcoord0Changed();
    m_dirty = true;
}

void TestGeometry::setTexcoord1(bool enable)
{
    if (enable == m_texcoord1)
        return;
    m_texcoord1 = enable;
    emit texcoord1Changed();
    m_dirty = true;
}

void TestGeometry::setTangent(bool enable)
{
    if (enable == m_tangent)
        return;
    m_tangent = enable;
    emit tangentChanged();
    m_dirty = true;
}

void TestGeometry::setBinormal(bool enable)
{
    if (enable == m_binormal)
        return;
    m_binormal = enable;
    emit binormalChanged();
    m_dirty = true;
}

void TestGeometry::setColor(bool enable)
{
    if (enable == m_color)
        return;
    m_color = enable;
    emit colorChanged();
    m_dirty = true;
}

QSSGRenderGraphObject *TestGeometry::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (m_dirty) {
        QVector3D min, max;
        int count = 120;
        QByteArray positionData = generatePositions(count, m_indexBuffer, false, min, max, 100.0f / float(count));
        QByteArray normalData;
        QByteArray uv0Data;
        QByteArray uv1Data;
        QByteArray colorData;
        QByteArray tangentData;
        QByteArray binormalData;
        clear();
        addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0,
                     QQuick3DGeometry::Attribute::ComponentType::F32Type);

        addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0,
                     QQuick3DGeometry::Attribute::ComponentType::U32Type);
        if (normal()) {
            normalData = generateNormals(positionData, m_indexBuffer);
            addAttribute(QQuick3DGeometry::Attribute::NormalSemantic, offsetof(Vertex, normal),
                         QQuick3DGeometry::Attribute::ComponentType::F32Type);
        }
        if (texcoord0()) {
            uv0Data = generateTexcoords0(positionData, normalData, min, max, 0.1f);
            addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic, offsetof(Vertex, uv0),
                         QQuick3DGeometry::Attribute::ComponentType::F32Type);
        }
        if (texcoord1()) {
            uv1Data = generateTexcoords1(positionData);
            addAttribute(QQuick3DGeometry::Attribute::TexCoord1Semantic, offsetof(Vertex, uv1),
                         QQuick3DGeometry::Attribute::ComponentType::F32Type);
        }
        if (tangent()) {
            tangentData = generateTangents(positionData, normalData, m_indexBuffer);
            addAttribute(QQuick3DGeometry::Attribute::TangentSemantic, offsetof(Vertex, tangent),
                         QQuick3DGeometry::Attribute::ComponentType::F32Type);
        }
        if (binormal()) {
            binormalData = generateBinormals(normalData, tangentData);
            addAttribute(QQuick3DGeometry::Attribute::BinormalSemantic, offsetof(Vertex, binormal),
                         QQuick3DGeometry::Attribute::ComponentType::F32Type);
        }
        if (color()) {
            colorData = generateColors(positionData);
            addAttribute(QQuick3DGeometry::Attribute::ColorSemantic, offsetof(Vertex, color),
                         QQuick3DGeometry::Attribute::ComponentType::F32Type);
        }
        packVertices(positionData, normalData, uv0Data, uv1Data, tangentData, binormalData, colorData, m_vertexBuffer);
        setStride(s_vertexSize);
        setVertexData(m_vertexBuffer);
        setIndexData(m_indexBuffer);
        setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
        setBounds(min, max);
        m_dirty = false;
    }
    node = QQuick3DGeometry::updateSpatialNode(node);
    return node;
}


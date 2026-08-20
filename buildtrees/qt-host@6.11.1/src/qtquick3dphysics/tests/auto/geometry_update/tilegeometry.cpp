// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tilegeometry.h"
#include <QtGui/QVector3D>
#include <array>

struct QObjMesh
{
    std::vector<unsigned int> indices;
    std::vector<QVector3D> vertices;
    std::vector<QVector2D> uvs;
};

QObjMesh calcUVs(const QObjMesh &obj)
{
    QObjMesh result = obj;
    QVector3D min = QVector3D(123456789.f, 123456789.f, 123456789.f);
    QVector3D max = QVector3D(-123456789.f, -123456789.f, -123456789.f);

    for (const auto &v : obj.vertices) {
        min.setX(qMin(v.x(), min.x()));
        min.setY(qMin(v.y(), min.y()));
        min.setZ(qMin(v.z(), min.z()));
        max.setX(qMax(v.x(), max.x()));
        max.setY(qMax(v.y(), max.y()));
        max.setZ(qMax(v.z(), max.z()));
    }

    QVector3D len = (max - min);

    for (const auto &v : obj.vertices) {
        QVector3D vmin = (v - min);
        result.uvs.push_back(QVector2D(vmin.x() / len.x(), vmin.z() / len.z()));
    }

    return result;
}

QObjMesh getHoleMesh()
{
    QObjMesh mesh;
    mesh.vertices = {
        QVector3D(-50, 0, 50),   QVector3D(-50, 0, 25),   QVector3D(-21, 0, 21),   QVector3D(-25, 0, 50),
        QVector3D(-25, 0, 50),   QVector3D(-21, 0, 21),   QVector3D(0, 0, 30),     QVector3D(0, 0, 50),
        QVector3D(0, 0, 50),     QVector3D(0, 0, 30),     QVector3D(21, 0, 21),    QVector3D(25, 0, 50),
        QVector3D(25, 0, 50),    QVector3D(21, 0, 21),    QVector3D(50, 0, 25),    QVector3D(50, 0, 50),
        QVector3D(-50, 0, 25),   QVector3D(-50, 0, 0),    QVector3D(-30, 0, 0),    QVector3D(-21, 0, 21),
        QVector3D(21, 0, 21),    QVector3D(30, 0, 0),     QVector3D(50, 0, 0),     QVector3D(50, 0, 25),
        QVector3D(-50, 0, 0),    QVector3D(-50, 0, -25),  QVector3D(-21, 0, -21),  QVector3D(-30, 0, 0),
        QVector3D(30, 0, 0),     QVector3D(21, 0, -21),   QVector3D(50, 0, -25),   QVector3D(50, 0, 0),
        QVector3D(-50, 0, -25),  QVector3D(-50, 0, -50),  QVector3D(-25, 0, -50),  QVector3D(-21, 0, -21),
        QVector3D(-21, 0, -21),  QVector3D(-25, 0, -50),  QVector3D(0, 0, -50),    QVector3D(0, 0, -30),
        QVector3D(0, 0, -30),    QVector3D(0, 0, -50),    QVector3D(25, 0, -50),   QVector3D(21, 0, -21),
        QVector3D(21, 0, -21),   QVector3D(25, 0, -50),   QVector3D(50, 0, -50),   QVector3D(50, 0, -25),
        QVector3D(-50, 10, 50),  QVector3D(-25, 10, 50),  QVector3D(-21, 10, 21),  QVector3D(-50, 10, 25),
        QVector3D(-25, 10, 50),  QVector3D(0, 10, 50),    QVector3D(0, 10, 30),    QVector3D(-21, 10, 21),
        QVector3D(0, 10, 50),    QVector3D(25, 10, 50),   QVector3D(21, 10, 21),   QVector3D(0, 10, 30),
        QVector3D(25, 10, 50),   QVector3D(50, 10, 50),   QVector3D(50, 10, 25),   QVector3D(21, 10, 21),
        QVector3D(-50, 10, 25),  QVector3D(-21, 10, 21),  QVector3D(-30, 10, 0),   QVector3D(-50, 10, 0),
        QVector3D(21, 10, 21),   QVector3D(50, 10, 25),   QVector3D(50, 10, 0),    QVector3D(30, 10, 0),
        QVector3D(-50, 10, 0),   QVector3D(-30, 10, 0),   QVector3D(-21, 10, -21), QVector3D(-50, 10, -25),
        QVector3D(30, 10, 0),    QVector3D(50, 10, 0),    QVector3D(50, 10, -25),  QVector3D(21, 10, -21),
        QVector3D(-50, 10, -25), QVector3D(-21, 10, -21), QVector3D(-25, 10, -50), QVector3D(-50, 10, -50),
        QVector3D(-21, 10, -21), QVector3D(0, 10, -30),   QVector3D(0, 10, -50),   QVector3D(-25, 10, -50),
        QVector3D(0, 10, -30),   QVector3D(21, 10, -21),  QVector3D(25, 10, -50),  QVector3D(0, 10, -50),
        QVector3D(21, 10, -21),  QVector3D(50, 10, -25),  QVector3D(50, 10, -50),  QVector3D(25, 10, -50),
        QVector3D(30, 0, 0),     QVector3D(21, 0, 21),    QVector3D(21, 10, 21),   QVector3D(30, 10, 0),
        QVector3D(21, 0, 21),    QVector3D(0, 0, 30),     QVector3D(0, 10, 30),    QVector3D(21, 10, 21),
        QVector3D(0, 0, -30),    QVector3D(21, 0, -21),   QVector3D(21, 10, -21),  QVector3D(0, 10, -30),
        QVector3D(-30, 0, 0),    QVector3D(-21, 0, -21),  QVector3D(-21, 10, -21), QVector3D(-30, 10, 0),
        QVector3D(0, 0, 30),     QVector3D(-21, 0, 21),   QVector3D(-21, 10, 21),  QVector3D(0, 10, 30),
        QVector3D(-21, 0, 21),   QVector3D(-30, 0, 0),    QVector3D(-30, 10, 0),   QVector3D(-21, 10, 21),
        QVector3D(-21, 0, -21),  QVector3D(0, 0, -30),    QVector3D(0, 10, -30),   QVector3D(-21, 10, -21),
        QVector3D(21, 0, -21),   QVector3D(30, 0, 0),     QVector3D(30, 10, 0),    QVector3D(21, 10, -21)
    };

    mesh.indices = { 0,   1,   3,   1,   2,   3,   4,   5,   7,   5,   6,   7,   8,   9,   11,  9,   10,  11,  12,  13,
                     15,  13,  14,  15,  16,  17,  19,  17,  18,  19,  20,  21,  23,  21,  22,  23,  24,  25,  27,  25,
                     26,  27,  28,  29,  31,  29,  30,  31,  32,  33,  35,  33,  34,  35,  36,  37,  39,  37,  38,  39,
                     40,  41,  43,  41,  42,  43,  44,  45,  47,  45,  46,  47,  48,  49,  51,  49,  50,  51,  52,  53,
                     55,  53,  54,  55,  56,  57,  59,  57,  58,  59,  60,  61,  63,  61,  62,  63,  64,  65,  67,  65,
                     66,  67,  68,  69,  71,  69,  70,  71,  72,  73,  75,  73,  74,  75,  76,  77,  79,  77,  78,  79,
                     80,  81,  83,  81,  82,  83,  84,  85,  87,  85,  86,  87,  88,  89,  91,  89,  90,  91,  92,  93,
                     95,  93,  94,  95,  96,  97,  99,  97,  98,  99,  100, 101, 103, 101, 102, 103, 104, 105, 107, 105,
                     106, 107, 108, 109, 111, 109, 110, 111, 112, 113, 115, 113, 114, 115, 116, 117, 119, 117, 118, 119,
                     120, 121, 123, 121, 122, 123, 124, 125, 127, 125, 126, 127 };

    return mesh;
}

QObjMesh getFloorMesh()
{
    QObjMesh mesh;
    mesh.vertices = {
        QVector3D(-50, 0, 50),   QVector3D(-50, 0, 25),   QVector3D(-25, 0, 25),   QVector3D(-25, 0, 50),
        QVector3D(-25, 0, 50),   QVector3D(-25, 0, 25),   QVector3D(0, 0, 25),     QVector3D(0, 0, 50),
        QVector3D(0, 0, 50),     QVector3D(0, 0, 25),     QVector3D(25, 0, 25),    QVector3D(25, 0, 50),
        QVector3D(25, 0, 50),    QVector3D(25, 0, 25),    QVector3D(50, 0, 25),    QVector3D(50, 0, 50),
        QVector3D(-50, 0, 25),   QVector3D(-50, 0, 0),    QVector3D(-25, 0, 0),    QVector3D(-25, 0, 25),
        QVector3D(-25, 0, 25),   QVector3D(-25, 0, 0),    QVector3D(0, 0, 0),      QVector3D(0, 0, 25),
        QVector3D(0, 0, 25),     QVector3D(0, 0, 0),      QVector3D(25, 0, 0),     QVector3D(25, 0, 25),
        QVector3D(25, 0, 25),    QVector3D(25, 0, 0),     QVector3D(50, 0, 0),     QVector3D(50, 0, 25),
        QVector3D(-50, 0, 0),    QVector3D(-50, 0, -25),  QVector3D(-25, 0, -25),  QVector3D(-25, 0, 0),
        QVector3D(-25, 0, 0),    QVector3D(-25, 0, -25),  QVector3D(0, 0, -25),    QVector3D(0, 0, 0),
        QVector3D(0, 0, 0),      QVector3D(0, 0, -25),    QVector3D(25, 0, -25),   QVector3D(25, 0, 0),
        QVector3D(25, 0, 0),     QVector3D(25, 0, -25),   QVector3D(50, 0, -25),   QVector3D(50, 0, 0),
        QVector3D(-50, 0, -25),  QVector3D(-50, 0, -50),  QVector3D(-25, 0, -50),  QVector3D(-25, 0, -25),
        QVector3D(-25, 0, -25),  QVector3D(-25, 0, -50),  QVector3D(0, 0, -50),    QVector3D(0, 0, -25),
        QVector3D(0, 0, -25),    QVector3D(0, 0, -50),    QVector3D(25, 0, -50),   QVector3D(25, 0, -25),
        QVector3D(25, 0, -25),   QVector3D(25, 0, -50),   QVector3D(50, 0, -50),   QVector3D(50, 0, -25),
        QVector3D(-50, 10, 50),  QVector3D(-25, 10, 50),  QVector3D(-25, 10, 25),  QVector3D(-50, 10, 25),
        QVector3D(-25, 10, 50),  QVector3D(0, 10, 50),    QVector3D(0, 10, 25),    QVector3D(-25, 10, 25),
        QVector3D(0, 10, 50),    QVector3D(25, 10, 50),   QVector3D(25, 10, 25),   QVector3D(0, 10, 25),
        QVector3D(25, 10, 50),   QVector3D(50, 10, 50),   QVector3D(50, 10, 25),   QVector3D(25, 10, 25),
        QVector3D(-50, 10, 25),  QVector3D(-25, 10, 25),  QVector3D(-25, 10, 0),   QVector3D(-50, 10, 0),
        QVector3D(-25, 10, 25),  QVector3D(0, 10, 25),    QVector3D(0, 10, 0),     QVector3D(-25, 10, 0),
        QVector3D(0, 10, 25),    QVector3D(25, 10, 25),   QVector3D(25, 10, 0),    QVector3D(0, 10, 0),
        QVector3D(25, 10, 25),   QVector3D(50, 10, 25),   QVector3D(50, 10, 0),    QVector3D(25, 10, 0),
        QVector3D(-50, 10, 0),   QVector3D(-25, 10, 0),   QVector3D(-25, 10, -25), QVector3D(-50, 10, -25),
        QVector3D(-25, 10, 0),   QVector3D(0, 10, 0),     QVector3D(0, 10, -25),   QVector3D(-25, 10, -25),
        QVector3D(0, 10, 0),     QVector3D(25, 10, 0),    QVector3D(25, 10, -25),  QVector3D(0, 10, -25),
        QVector3D(25, 10, 0),    QVector3D(50, 10, 0),    QVector3D(50, 10, -25),  QVector3D(25, 10, -25),
        QVector3D(-50, 10, -25), QVector3D(-25, 10, -25), QVector3D(-25, 10, -50), QVector3D(-50, 10, -50),
        QVector3D(-25, 10, -25), QVector3D(0, 10, -25),   QVector3D(0, 10, -50),   QVector3D(-25, 10, -50),
        QVector3D(0, 10, -25),   QVector3D(25, 10, -25),  QVector3D(25, 10, -50),  QVector3D(0, 10, -50),
        QVector3D(25, 10, -25),  QVector3D(50, 10, -25),  QVector3D(50, 10, -50),  QVector3D(25, 10, -50)
    };

    mesh.indices = { 0,   1,   3,   1,   2,   3,   4,   5,   7,   5,   6,   7,   8,   9,   11,  9,   10,  11,  12,  13,
                     15,  13,  14,  15,  16,  17,  19,  17,  18,  19,  20,  21,  23,  21,  22,  23,  24,  25,  27,  25,
                     26,  27,  28,  29,  31,  29,  30,  31,  32,  33,  35,  33,  34,  35,  36,  37,  39,  37,  38,  39,
                     40,  41,  43,  41,  42,  43,  44,  45,  47,  45,  46,  47,  48,  49,  51,  49,  50,  51,  52,  53,
                     55,  53,  54,  55,  56,  57,  59,  57,  58,  59,  60,  61,  63,  61,  62,  63,  64,  65,  67,  65,
                     66,  67,  68,  69,  71,  69,  70,  71,  72,  73,  75,  73,  74,  75,  76,  77,  79,  77,  78,  79,
                     80,  81,  83,  81,  82,  83,  84,  85,  87,  85,  86,  87,  88,  89,  91,  89,  90,  91,  92,  93,
                     95,  93,  94,  95,  96,  97,  99,  97,  98,  99,  100, 101, 103, 101, 102, 103, 104, 105, 107, 105,
                     106, 107, 108, 109, 111, 109, 110, 111, 112, 113, 115, 113, 114, 115, 116, 117, 119, 117, 118, 119,
                     120, 121, 123, 121, 122, 123, 124, 125, 127, 125, 126, 127 };

    return mesh;
}

void TileGeometry::createFloor()
{
    auto mesh = calcUVs(m_hasHole ? getHoleMesh() : getFloorMesh());

    // pos + uv
    constexpr int kStride = sizeof(QVector3D) + sizeof(QVector2D);
    QByteArray vertexData(mesh.vertices.size() * kStride, Qt::Initialization::Uninitialized);

    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        QVector3D *vertex = reinterpret_cast<QVector3D *>(vertexData.data() + (kStride * i));
        QVector2D *uv = reinterpret_cast<QVector2D *>(vertexData.data() + (kStride * i) + sizeof(QVector3D));
        *vertex = mesh.vertices[i];
        *uv = mesh.uvs[i];
    }

    QByteArray indexBuffer(mesh.indices.size() * sizeof(unsigned int), Qt::Initialization::Uninitialized);
    memcpy(indexBuffer.data(), mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));

    clear();
    setVertexData(vertexData);
    setStride(kStride);
    setBounds(QVector3D(-50.0f, -50.0f, -50.0f), QVector3D(+50.0f, +50.0f, +50.0f));
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic, sizeof(QVector3D), QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic, 0, QQuick3DGeometry::Attribute::U32Type);
    setIndexData(indexBuffer);
    update();
}

TileGeometry::TileGeometry()
{
    createFloor();
}

void TileGeometry::setHasHole(bool v)
{
    if (m_hasHole == v)
        return;
    m_hasHole = v;
    createFloor();
    emit hasHoleChanged(v);
}

bool TileGeometry::hasHole()
{
    return m_hasHole;
}

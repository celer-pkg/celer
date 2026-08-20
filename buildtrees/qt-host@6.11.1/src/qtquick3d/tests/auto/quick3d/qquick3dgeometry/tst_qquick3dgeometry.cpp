// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>

#include <QtQuick3D/private/qquick3dgeometry_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendergeometry_p.h>
#include <QtQuick3DUtils/private/qssgmesh_p.h>

class tst_QQuick3DGeometry : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Geometry : public QQuick3DGeometry
    {
    public:
        using QQuick3DGeometry::updateSpatialNode;
    };

private slots:
    void testGeometry();
    void testGeometry2();
    void testPartialUpdate();
    void testGeometrySubset();
};

void tst_QQuick3DGeometry::testGeometry()
{
    QSSGRenderGeometry geom;

    QByteArray vertexData;
    vertexData.resize(100);
    vertexData.fill(0);
    geom.setVertexData(vertexData);
    QCOMPARE(vertexData.size(), geom.vertexBuffer().size());
    QVERIFY(geom.vertexBuffer().compare(vertexData) == 0);

    QByteArray indexData;
    indexData.resize(100);
    indexData.fill(0);
    geom.setIndexData(indexData);
    QCOMPARE(indexData.size(), geom.indexBuffer().size());
    QVERIFY(geom.indexBuffer().compare(indexData) == 0);

    const int stride = 20;
    geom.setStride(stride);
    QCOMPARE(stride, geom.stride());

    const QVector3D minimum(-10.f, -10.f, -10.f);
    const QVector3D maximum(10.f, 10.f, 10.f);
    geom.setBounds(minimum, maximum);
    QVERIFY(qFuzzyCompare(minimum, geom.boundsMin()));
    QVERIFY(qFuzzyCompare(maximum, geom.boundsMax()));

    geom.clear();
    QSSGBounds3 emptyBounds;
    QCOMPARE(geom.vertexBuffer().size(), 0);
    QCOMPARE(geom.indexBuffer().size(), 0);
    QVERIFY(qFuzzyCompare(emptyBounds.minimum, geom.boundsMin()));
    QVERIFY(qFuzzyCompare(emptyBounds.maximum, geom.boundsMax()));
    QCOMPARE(geom.attributeCount(), 0);

    const QSSGMesh::Mesh::DrawMode primitiveTypes[] = {
        QSSGMesh::Mesh::DrawMode::Points,
        QSSGMesh::Mesh::DrawMode::LineStrip,
        QSSGMesh::Mesh::DrawMode::LineLoop,
        QSSGMesh::Mesh::DrawMode::Lines,
        QSSGMesh::Mesh::DrawMode::TriangleStrip,
        QSSGMesh::Mesh::DrawMode::TriangleFan,
        QSSGMesh::Mesh::DrawMode::Triangles
    };
    for (const auto primitiveType : primitiveTypes) {
        geom.setPrimitiveType(primitiveType);
        QCOMPARE(primitiveType, geom.primitiveType());
    }

    const QSSGMesh::RuntimeMeshData::Attribute::Semantic semantics[] = {
        QSSGMesh::RuntimeMeshData::Attribute::IndexSemantic,
        QSSGMesh::RuntimeMeshData::Attribute::PositionSemantic,
        QSSGMesh::RuntimeMeshData::Attribute::NormalSemantic,
        QSSGMesh::RuntimeMeshData::Attribute::TexCoordSemantic,
        QSSGMesh::RuntimeMeshData::Attribute::TangentSemantic,
        QSSGMesh::RuntimeMeshData::Attribute::BinormalSemantic,
    };

    const int offsets[] = {
        0, 16, 21, 33, 46, 52,
    };

    const QSSGMesh::Mesh::ComponentType types[] = {
        QSSGMesh::Mesh::ComponentType::UnsignedInt8,
        QSSGMesh::Mesh::ComponentType::Int8,
        QSSGMesh::Mesh::ComponentType::UnsignedInt16,
        QSSGMesh::Mesh::ComponentType::Int16,
        QSSGMesh::Mesh::ComponentType::UnsignedInt32,
        QSSGMesh::Mesh::ComponentType::Int32,
        QSSGMesh::Mesh::ComponentType::UnsignedInt64,
        QSSGMesh::Mesh::ComponentType::Int64,
        QSSGMesh::Mesh::ComponentType::Float16,
        QSSGMesh::Mesh::ComponentType::Float32,
        QSSGMesh::Mesh::ComponentType::Float64
    };

    for (int i = 0; i < 6; i++) {
        geom.addAttribute(semantics[i], offsets[i], QSSGMesh::Mesh::ComponentType::Float32);
        QCOMPARE(semantics[i], geom.attribute(i).semantic);
        QCOMPARE(offsets[i], geom.attribute(i).offset);
        QCOMPARE(geom.attributeCount(), i+1);
    }
    QCOMPARE(geom.attributeCount(), 6);
    geom.clear();
    QCOMPARE(geom.attributeCount(), 0);

    for (auto componentType : types) {
        geom.addAttribute(QSSGMesh::RuntimeMeshData::Attribute::PositionSemantic, 0, componentType);
        QCOMPARE(componentType, geom.attribute(0).componentType);
        geom.clear();
    }
}


void tst_QQuick3DGeometry::testGeometry2()
{
    Geometry geom;

    QByteArray vertexData;
    vertexData.resize(100);
    vertexData.fill(0);
    geom.setVertexData(vertexData);
    QCOMPARE(vertexData.size(), geom.vertexData().size());
    QVERIFY(geom.vertexData().compare(vertexData) == 0);

    QByteArray indexData;
    indexData.resize(100);
    indexData.fill(0);
    geom.setIndexData(indexData);
    QCOMPARE(indexData.size(), geom.indexData().size());
    QVERIFY(geom.indexData().compare(indexData) == 0);

    const int stride = 20;
    geom.setStride(stride);
    QCOMPARE(stride, geom.stride());

    const QVector3D minimum(-10.f, -10.f, -10.f);
    const QVector3D maximum(10.f, 10.f, 10.f);
    geom.setBounds(minimum, maximum);
    QVERIFY(qFuzzyCompare(minimum, geom.boundsMin()));
    QVERIFY(qFuzzyCompare(maximum, geom.boundsMax()));

    geom.clear();
    QCOMPARE(geom.vertexData().size(), 0);
    QCOMPARE(geom.indexData().size(), 0);
    QCOMPARE(geom.attributeCount(), 0);

    const QQuick3DGeometry::PrimitiveType primitiveTypes[] = {
        QQuick3DGeometry::PrimitiveType::Points,
        QQuick3DGeometry::PrimitiveType::LineStrip,
        QQuick3DGeometry::PrimitiveType::Lines,
        QQuick3DGeometry::PrimitiveType::TriangleStrip,
        QQuick3DGeometry::PrimitiveType::TriangleFan,
        QQuick3DGeometry::PrimitiveType::Triangles,
    };
    for (const auto primitiveType : primitiveTypes) {
        geom.setPrimitiveType(primitiveType);
        QCOMPARE(primitiveType, geom.primitiveType());
    }

    const QQuick3DGeometry::Attribute::Semantic semantics[] = {
        QQuick3DGeometry::Attribute::IndexSemantic,
        QQuick3DGeometry::Attribute::PositionSemantic,
        QQuick3DGeometry::Attribute::NormalSemantic,
        QQuick3DGeometry::Attribute::TexCoordSemantic,
        QQuick3DGeometry::Attribute::TangentSemantic,
        QQuick3DGeometry::Attribute::BinormalSemantic,
    };

    const int offsets[] = {
        0, 16, 21, 33, 46, 52,
    };

    const QQuick3DGeometry::Attribute::ComponentType types[] = {
        QQuick3DGeometry::Attribute::U16Type,
        QQuick3DGeometry::Attribute::U32Type,
        QQuick3DGeometry::Attribute::F32Type,
    };

    for (int i = 0; i < 6; i++) {
        geom.addAttribute(semantics[i], offsets[i], QQuick3DGeometry::Attribute::F32Type);
        QCOMPARE(semantics[i], geom.attribute(i).semantic);
        QCOMPARE(offsets[i], geom.attribute(i).offset);
        QCOMPARE(geom.attributeCount(), i+1);
    }
    QCOMPARE(geom.attributeCount(), 6);
    geom.clear();
    QCOMPARE(geom.attributeCount(), 0);

    for (auto componentType : types) {
        geom.addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, componentType);
        QCOMPARE(componentType, geom.attribute(0).componentType);
        geom.clear();
    }
}

void tst_QQuick3DGeometry::testPartialUpdate()
{
    Geometry geom;

    QByteArray vertexData(100, 'a');
    geom.setVertexData(vertexData);
    QCOMPARE(vertexData.size(), geom.vertexData().size());
    QCOMPARE(geom.vertexData(), vertexData);

    QByteArray indexData(100, 'a');
    geom.setIndexData(indexData);
    QCOMPARE(indexData.size(), geom.indexData().size());
    QCOMPARE(geom.indexData(), indexData);

    QByteArray smallData(10, 'b');
    geom.setVertexData(30, smallData);
    QCOMPARE(geom.vertexData().size(), 100);
    QCOMPARE(geom.vertexData().left(30), QByteArray(30, 'a'));
    QCOMPARE(geom.vertexData().mid(30, 10), smallData);
    QCOMPARE(geom.vertexData().mid(40), QByteArray(60, 'a'));

    geom.setIndexData(40, smallData);
    QCOMPARE(geom.indexData().size(), 100);
    QCOMPARE(geom.indexData().left(40), QByteArray(40, 'a'));
    QCOMPARE(geom.indexData().mid(40, 10), smallData);
    QCOMPARE(geom.indexData().mid(50), QByteArray(50, 'a'));

    geom.setVertexData(100, smallData);
    QCOMPARE(geom.vertexData().size(), 100);
    geom.setVertexData(101, smallData);
    QCOMPARE(geom.vertexData().size(), 100);

    geom.setIndexData(100, smallData);
    QCOMPARE(geom.indexData().size(), 100);
    geom.setIndexData(101, smallData);
    QCOMPARE(geom.indexData().size(), 100);

    geom.setVertexData(95, smallData);
    QCOMPARE(geom.vertexData().size(), 100);
    QCOMPARE(geom.vertexData().mid(45, 50), QByteArray(50, 'a'));
    QCOMPARE(geom.vertexData().mid(95), smallData.left(5));

    geom.setIndexData(95, smallData);
    QCOMPARE(geom.indexData().size(), 100);
    QCOMPARE(geom.indexData().mid(55, 40), QByteArray(40, 'a'));
    QCOMPARE(geom.indexData().mid(95), smallData.left(5));
}

void tst_QQuick3DGeometry::testGeometrySubset()
{
    Geometry geom;
    QCOMPARE(geom.subsetCount(), 0);

    geom.addSubset(0, 50, QVector3D(-10, -10, -10), QVector3D(0, 0, 0), "subset1");
    QCOMPARE(geom.subsetCount(), 1);

    geom.addSubset(50, 50, QVector3D(0, 0, 0), QVector3D(10, 10, 10), "subset2");
    QCOMPARE(geom.subsetCount(), 2);

    QCOMPARE(geom.subsetOffset(0), 0);
    QCOMPARE(geom.subsetCount(0), 50);
    QVERIFY(qFuzzyCompare(geom.subsetBoundsMin(0), QVector3D(-10, -10, -10)));
    QVERIFY(qFuzzyCompare(geom.subsetBoundsMax(0), QVector3D(0, 0, 0)));
    QCOMPARE(geom.subsetName(0), "subset1");

    QCOMPARE(geom.subsetOffset(1), 50);
    QCOMPARE(geom.subsetCount(1), 50);
    QVERIFY(qFuzzyCompare(geom.subsetBoundsMin(1), QVector3D(0, 0, 0)));
    QVERIFY(qFuzzyCompare(geom.subsetBoundsMax(1), QVector3D(10, 10, 10)));
    QCOMPARE(geom.subsetName(1), "subset2");
}

QTEST_APPLESS_MAIN(tst_QQuick3DGeometry)
#include "tst_qquick3dgeometry.moc"

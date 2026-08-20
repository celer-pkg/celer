// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtQuick3DUtils/private/qssgmeshbvh_p.h>
#include <QtQuick3DUtils/private/qssgmeshbvhbuilder_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendergeometry_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>

class Bvh : public QObject
{
    Q_OBJECT
public:
    Bvh() = default;
    ~Bvh() = default;

    struct Attributes
    {
        QSSGMesh::RuntimeMeshData::Attribute::Semantic semantic;
        int offset;
        QSSGMesh::Mesh::ComponentType componentType;
    };

    struct Bounds
    {
        QVector3D min {};
        QVector3D max {};
    };

private Q_SLOTS:
    void bench_bvh_small();
    void bench_bvh_medium();
    void bench_bvh_large();

private:
    void prepRenderGeometry(qsizetype width, qsizetype height, QSSGRenderGeometry &renderGeometry);
};

void Bvh::bench_bvh_small()
{
    const qsizetype width = 128;
    const qsizetype height = 128;

    QSSGRenderGeometry renderGeometry;
    prepRenderGeometry(width, height, renderGeometry);

    QVarLengthArray<std::unique_ptr<QSSGMeshBVH>, 4096> bvhStorage;
    QBENCHMARK {
         bvhStorage.push_back(QSSGBufferManager::loadMeshBVH(&renderGeometry));
    }

    for (const auto &bvh : bvhStorage)
         QVERIFY(bvh != nullptr && bvh->nodes().size() > 0);
}

void Bvh::bench_bvh_medium()
{
    const qsizetype width = 2048;
    const qsizetype height = 2048;

    QSSGRenderGeometry renderGeometry;
    prepRenderGeometry(width, height, renderGeometry);

    QVarLengthArray<std::unique_ptr<QSSGMeshBVH>, 2048> bvhStorage;
    QBENCHMARK {
         bvhStorage.push_back(QSSGBufferManager::loadMeshBVH(&renderGeometry));
    }

    for (const auto &bvh : bvhStorage)
         QVERIFY(bvh != nullptr && bvh->nodes().size() > 0);
}

void Bvh::bench_bvh_large()
{
// #define ENABLE_LARGE_TEST
#ifndef ENABLE_LARGE_TEST
    QSKIP("The size for the large test is resource intensive and can be platform specific");
#endif // ENABLE_LARGE_TEST
    const qsizetype width = 8192;
    const qsizetype height = 8192;

    QSSGRenderGeometry renderGeometry;
    prepRenderGeometry(width, height, renderGeometry);

    QVarLengthArray<std::unique_ptr<QSSGMeshBVH>, 1024> bvhStorage;
    QBENCHMARK {
         bvhStorage.push_back(QSSGBufferManager::loadMeshBVH(&renderGeometry));
    }

    for (const auto &bvh : bvhStorage)
         QVERIFY(bvh != nullptr && bvh->nodes().size() > 0);
}

void Bvh::prepRenderGeometry(qsizetype width, qsizetype height, QSSGRenderGeometry &renderGeometry)
{
    const qsizetype entries = width * height;

    Bounds bounds { {}, { float(width), float(height), 0 }};

    size_t stride = width * sizeof(QVector3D);

    QByteArray vertexBuffer;
    vertexBuffer.resize(stride * height);
    vertexBuffer.fill(0);

    QSSGDataRef<QVector3D> dataRef{ reinterpret_cast<QVector3D *>(vertexBuffer.data()), entries };
    for (auto h = 0; h != height; ++h) {
         for (auto w = 0; w != width; ++w)
             dataRef[(h * w) + w] = QVector3D{ float(w), float(h), 0.0f };
    }

    auto primitiveType = QSSGRenderDrawMode::Triangles;

    renderGeometry.clearVertexAndIndex();
    renderGeometry.setBounds(bounds.min, bounds.max);
    renderGeometry.setStride(stride);
    renderGeometry.setVertexData(vertexBuffer);
    renderGeometry.setPrimitiveType(primitiveType);

    Attributes attributes[] { { QSSGMesh::RuntimeMeshData::Attribute::PositionSemantic, 0, QSSGRenderComponentType::Float32 } };

    for (const auto &attr : attributes)
         renderGeometry.addAttribute(attr.semantic, attr.offset, attr.componentType);

    {
         quint32 offset = 0;
         quint32 count = 0;
         renderGeometry.addSubset(offset, count, bounds.min, bounds.max);
    }
}

QTEST_APPLESS_MAIN(Bvh)

#include "tst_bvh.moc"

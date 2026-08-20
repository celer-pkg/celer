// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QtQuick3DUtils/private/qssgbounds3_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderclippingfrustum_p.h>

#include <QtGui/qquaternion.h>

class BenchFrustumCulling : public QObject
{
    Q_OBJECT

public:
    BenchFrustumCulling();
    ~BenchFrustumCulling();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_frustumCulling();
    void bench_outputlist();
    void bench_inline();

private:
    struct ObjectData
    {
        QVector3D worldCenterPt;
        QMatrix4x4 globalTransform;
        QSSGBounds3 bounds;
    };

    static ObjectData createRenderableData(QVector3D position, QQuaternion rotation, QSSGBounds3 bounds)
    {
        auto transform = QSSGRenderNode::calculateTransformMatrix(position, {1.0f, 1.0f, 1.0f}, {}, rotation);
        return ObjectData{ {}, transform, bounds};
    }

    static void populateRenderableList(const QList<ObjectData> &objects, QList<QSSGRenderableObject *> &renderableObjects)
    {
        renderableObjects.clear();
        renderableObjects.reserve(objects.size());

        for (const auto &od : std::as_const(objects)) {
            QSSGBounds3 globalBounds = od.bounds;
            globalBounds.transform(od.globalTransform);
            // NOTE: We pass in the global bounds, as that's the only thing we care about here.
            // In practice we never create the base type QSSGRenderableObject in the engine.
            renderableObjects.push_back(new QSSGRenderableObject{ QSSGSubsetRenderable::Type::DefaultMaterialMeshSubset, QSSGRenderableObjectFlags(), od.worldCenterPt, globalBounds, 0.0f });
        }
    }

    QQuick3DPerspectiveCamera camera;
    QScopedPointer<QSSGRenderCamera> cameraNode;
    QSSGClippingFrustum clipFrustum;
};

BenchFrustumCulling::BenchFrustumCulling()
{

}

BenchFrustumCulling::~BenchFrustumCulling()
{

}

void BenchFrustumCulling::initTestCase()
{
    const QRect viewport = { 0, 0, 100, 100 };
    camera.setPosition({0, 0, 100});
    camera.setClipNear(95.0f);
    camera.setClipFar(105.0f);
    cameraNode.reset(static_cast<QSSGRenderCamera *>(QQuick3DObjectPrivate::updateSpatialNode(&camera, nullptr)));
    QSSGRenderCamera::calculateProjectionInternal(*cameraNode, viewport);

    QMatrix4x4 viewProjectionMatrix = QMatrix4x4(Qt::Uninitialized);
    QSSGClipPlane nearPlane;

    {
        auto &camera = *cameraNode;
        camera.calculateViewProjectionMatrix(camera.localTransform, viewProjectionMatrix);

        QMatrix3x3 theUpper33(camera.localTransform.normalMatrix());

        QVector3D dir(QSSGUtils::mat33::transform(theUpper33, QVector3D(0, 0, -1)));
        dir.normalize();
        nearPlane.normal = dir;
        QVector3D theGlobalPos = QSSGRenderNode::getGlobalPos(camera.localTransform) + camera.clipPlanes.clipNear() * dir;
        nearPlane.d = -(QVector3D::dotProduct(dir, theGlobalPos));
    }
    // the near plane's bbox edges are calculated in the clipping frustum's
    // constructor.

    clipFrustum = QSSGClippingFrustum(viewProjectionMatrix, nearPlane);
}

void BenchFrustumCulling::cleanupTestCase()
{

}

void BenchFrustumCulling::test_frustumCulling()
{

}

constexpr auto Identity = QQuaternion{1, 0, 0, 0};

void BenchFrustumCulling::bench_outputlist()
{
    // bounds 10x10x10 all in world coordinates
    constexpr float widthAndHeight = 10.0f;
    constexpr QSSGBounds3 bounds { { -widthAndHeight / 2.0f, -widthAndHeight / 2.0f, -widthAndHeight / 2.0f }, { widthAndHeight / 2.0f, widthAndHeight / 2.0f, widthAndHeight / 2.0f } };

    // For simplicity we only do put "cullable" object in front or behind the frustum for now.
    const float frustumNearBorder = camera.position().z() - camera.clipNear() + widthAndHeight;
    const float frustumFarBorder = camera.position().z() - camera.clipFar() - widthAndHeight;

    const quint32 objectCount = 10000;
    const quint32 nonCulledItemCount = 3;

    QSet<quint32> replaceIndexes;
    while (replaceIndexes.size() < nonCulledItemCount)
        replaceIndexes.insert(QRandomGenerator::global()->bounded(objectCount));

    QList<ObjectData> objects;
    objects.reserve(objectCount);

    // Fill the list with object data that should be culled
    for (quint32 i = 0, end = objectCount; i != end; ++i) {
        if (i % 2)
            objects.push_back(createRenderableData({0.0f, 0.0f, frustumNearBorder }, Identity, bounds));
        else
            objects.push_back(createRenderableData({0.0f, 0.0f, frustumFarBorder }, Identity, bounds));
    }

    // Insert items at random positions in the list that should not be culled
    for (auto v : std::as_const(replaceIndexes))
        objects.replace(v, createRenderableData({0.0f, 0.0f, 0.0f}, Identity, bounds));

    QCOMPARE(objects.size(), objectCount);

    QList<QSSGRenderableObject *> renderableObjects;

    // List of renderables
    populateRenderableList(objects, renderableObjects);

    // Renderable object handle class...
    QSSGRenderableObjectList renderables;
    renderables.reserve(objects.size());

    QSSGRenderableObjectList culledrenderables;
    renderables.reserve(objects.size());

    for (auto *ro : renderableObjects)
        renderables.push_back({ ro, 0.0f, {}});

    QVERIFY(!cameraNode->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));

    QBENCHMARK {
        culledrenderables.clear();
        QSSGLayerRenderData::frustumCulling(clipFrustum, renderables, culledrenderables);
    }

    QCOMPARE(culledrenderables.size(), nonCulledItemCount);
}

void BenchFrustumCulling::bench_inline()
{
    // bounds 10x10x10 all in world coordinates
    constexpr float widthAndHeight = 10.0f;
    constexpr QSSGBounds3 bounds { { -widthAndHeight / 2.0f, -widthAndHeight / 2.0f, -widthAndHeight / 2.0f }, { widthAndHeight / 2.0f, widthAndHeight / 2.0f, widthAndHeight / 2.0f } };

    // For simplicity we only do put "cullable" object in front or behind the frustum for now.
    const float frustumNearBorder = camera.position().z() - camera.clipNear() + widthAndHeight;
    const float frustumFarBorder = camera.position().z() - camera.clipFar() - widthAndHeight;

    const quint32 objectCount = 10000;
    const quint32 nonCulledItemCount = 3;

    QSet<quint32> replaceIndexes;
    while (replaceIndexes.size() < nonCulledItemCount)
        replaceIndexes.insert(QRandomGenerator::global()->bounded(objectCount));

    QList<ObjectData> objects;
    objects.reserve(objectCount);

    // Fill the list with object data that should be culled
    for (quint32 i = 0, end = objectCount; i != end; ++i) {
        if (i % 2)
            objects.push_back(createRenderableData({0.0f, 0.0f, frustumNearBorder }, Identity, bounds));
        else
            objects.push_back(createRenderableData({0.0f, 0.0f, frustumFarBorder }, Identity, bounds));
    }

    // Insert items at random positions in the list that should not be culled
    for (auto v : std::as_const(replaceIndexes))
        objects.replace(v, createRenderableData({0.0f, 0.0f, 0.0f}, Identity, bounds));

    QCOMPARE(objects.size(), objectCount);

    QList<QSSGRenderableObject *> renderableObjects;

    // List of renderables
    populateRenderableList(objects, renderableObjects);

    // Renderable object handle class...
    QSSGRenderableObjectList renderables;
    renderables.reserve(objects.size());

    for (auto *ro : renderableObjects)
        renderables.push_back({ ro, 0.0f, {} });

    QVERIFY(!cameraNode->isDirty(QSSGRenderCamera::DirtyFlag::CameraDirty));

    qsizetype ret;
    QBENCHMARK {
        ret = QSSGLayerRenderData::frustumCullingInline(clipFrustum, renderables);
    }


    QCOMPARE(ret, nonCulledItemCount);
}

QTEST_APPLESS_MAIN(BenchFrustumCulling)

#include "tst_benchfrustumculling.moc"

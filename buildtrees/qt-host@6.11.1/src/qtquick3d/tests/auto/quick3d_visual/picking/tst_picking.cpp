// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#include <QSignalSpy>
#include <QTest>
#include <QtQuick/QQuickItem>

#include <QtQuick3D/private/qquick3dviewport_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
#include <QtQuick3D/private/qquick3dpickresult_p.h>
#include <QtQuick3D/private/qquick3ditem2d_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderray_p.h>

#include "../shared/util.h"

class CustomGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    QML_NAMED_ELEMENT(CustomGeometry)

public:
    CustomGeometry(uint count);

    int count() { return m_count; }
    void setCount(int count);

signals:
    void countChanged();

private:
    struct Vertex {
        QVector3D position;
        QVector3D normal;
        QVector2D uv;
    };

    void updateData();

    uint m_count = 500;
};


CustomGeometry::CustomGeometry(uint count)
{
    m_count = count;
    updateData();
}

void CustomGeometry::setCount(int count)
{
    m_count = count;
    updateData();
    emit countChanged();
}

void CustomGeometry::updateData()
{
    QVector<Vertex> vertices;
    QVector<QPoint> coords;

    int rowCount = m_count;
    int columnCount = m_count;
    float uvX = 1.0f / float(rowCount - 1);
    float uvY = 1.0f / float(columnCount - 1);

    for (int i = 0; i < columnCount; i++) {
        for (int j = 0; j < rowCount; j++) {
            Vertex v;
            float x = 4.0f / (columnCount-1) * i - 2.0f;
            float z = 4.0f / (rowCount-1) * j - 2.0f;
            v.position = QVector3D(x, .0f, z);
            v.normal = QVector3D(.0f, .0f, .0f);
            v.uv = QVector2D(j * uvX, i * uvY);
            vertices.push_back(v);
            coords.push_back(QPoint(i, j));
        }
    }
    QVector3D boundsMin = QVector3D(-2.0f, -1.0f, -2.0f);
    QVector3D boundsMax = QVector3D(2.0f, 1.0f, 2.0f);
    setBounds(boundsMin, boundsMax);

    QVector<quint32> indices;
    for (int i = 0; i < columnCount - 1; i++) {
        for (int j = 0; j < rowCount -1; j++) {
            indices.push_back(i * rowCount + j);
            indices.push_back(i * rowCount + j + 1);
            indices.push_back(i * rowCount + j + rowCount);

            indices.push_back(i * rowCount + j + 1);
            indices.push_back(i * rowCount + j + 1 + rowCount);
            indices.push_back(i * rowCount + j + rowCount);
        }
    }

    setStride(sizeof(Vertex));
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                           0,
                           QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::TexCoord0Semantic,
                           sizeof(QVector3D) * 2,
                           QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                           sizeof(QVector3D),
                           QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                           0,
                           QQuick3DGeometry::Attribute::U32Type);

    QByteArray vertexBuffer(reinterpret_cast<char *>(vertices.data()), vertices.size() * sizeof(Vertex));
    setVertexData(vertexBuffer);

    QByteArray indexBuffer(reinterpret_cast<char *>(indices.data()), indices.size() * sizeof(quint32));
    setIndexData(indexBuffer);

    update();
}


class tst_Picking : public QQuick3DDataTest
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() override;
    void test_view_picking();
    void test_ray_picking();
    void test_object_picking2();
    void test_item_picking();
    void test_picking_QTBUG_111997();
    void test_picking_corner_case();
    void test_triangleIntersect();
    void test_sphere_geometry();
    void test_closest_point_picking();

private:
    QQuickItem *find2DChildIn3DNode(QQuickView *view, const QString &objectName, const QString &itemName);

};

void tst_Picking::initTestCase()
{
    QQuick3DDataTest::initTestCase();
    if (!initialized())
        return;
}

QQuickItem *tst_Picking::find2DChildIn3DNode(QQuickView *view, const QString &objectName, const QString &itemName)
{
    QQuick3DNode *obj = view->rootObject()->findChild<QQuick3DNode*>(objectName);
    if (!obj)
        return nullptr;
    QQuickItem *subsceneRoot = obj->findChild<QQuickItem *>();
    if (!subsceneRoot)
        subsceneRoot = obj->findChild<QQuick3DItem2D *>()->contentItem();
    if (!subsceneRoot)
        return nullptr;
    QObject *child = subsceneRoot->findChild<QObject *>(itemName);
    return static_cast<QQuickItem *>(child);
}

void tst_Picking::test_view_picking()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("picking.qml"), QSize(400, 400)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *model1 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model1"));
    QVERIFY(model1);
    QQuick3DModel *model2 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model2"));
    QVERIFY(model2);
    QQuick3DModel *instancedModel = view3d->findChild<QQuick3DModel *>(QStringLiteral("instancedModel"));
    QVERIFY(instancedModel);
    QQuickItem *item2d = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dNode", "item2d"));
    QVERIFY(item2d);

    const qreal dpr = view->devicePixelRatio();
    if (dpr != 1.0) {
        QSKIP("Test uses window positions to get exact values and those assume DPR of 1.0");
    }

    // Pick nearest based on viewport position

    // Center of model1
    auto result = view3d->pick(200, 200);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model1);
    QCOMPARE(result.distance(), 550.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.5f, 0.5f));
    QCOMPARE(result.scenePosition(), QVector3D(0.0f, 0.0f, 50.0f));
    QCOMPARE(result.position(), QVector3D(0.0f, 0.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Upper right corner of model1
    result = view3d->pick(250, 150);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model1);
    QCOMPARE(result.distance(), 550.0f);
    QCOMPARE(result.uvPosition(), QVector2D(1.0f, 1.0f));
    QCOMPARE(result.scenePosition(), QVector3D(50.0f, 50.0f, 50.0f));
    QCOMPARE(result.position(), QVector3D(50.0f, 50.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Just outside model1's upper right corner, so should hit the model behind (model2)
    // This one might cause some floating point grief
    result = view3d->pick(251, 151);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model2);
    QCOMPARE(result.distance(), 600.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.5099999904632568, 0.4899999797344208));
    QCOMPARE(result.scenePosition(), QVector3D(51.0f, 49.0f, -0.000003814697265625));
    QCOMPARE(result.position(), QVector3D(1.0f, -1.0f, 49.999996185302734f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Upper right corner of model2
    result = view3d->pick(300, 100);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model2);
    QCOMPARE(result.distance(), 600.0f);
    QCOMPARE(result.uvPosition(), QVector2D(1.0f, 1.0f));
    QCOMPARE(result.scenePosition(), QVector3D(100.0f, 100.0f, 0.0f));
    QCOMPARE(result.position(), QVector3D(50.0f, 50.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Just outside model2's upper right corner, so there should be no hit
    result = view3d->pick(301, 99);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QCOMPARE(result.objectHit(), nullptr);
    QCOMPARE(result.itemHit(), nullptr);

    // Center of the third entry in the instance table
    result = view3d->pick(350, 200);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), instancedModel);
    QCOMPARE(result.distance(), 550.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.0f, 0.5f));
    QCOMPARE(result.scenePosition(), QVector3D(150.0f, 0.0f, 50.0f));
    QCOMPARE(result.position(), QVector3D(-50.0f, 0.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 2);
    QCOMPARE(result.itemHit(), nullptr);

    // The bottom right of instancedModel,
    // overlapped by model1 and then model2, should hit model1
    result = view3d->pick(225, 175);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model1);
    QCOMPARE(result.distance(), 550.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.75f, 0.75f));
    QCOMPARE(result.scenePosition(), QVector3D(25.0f, 25.0f, 50.0f));
    QCOMPARE(result.position(), QVector3D(25.0f, 25.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Pick one specific based on viewport position

    // Center of model1, overlapping model2,
    // only check for model2, so should hit model2
    result = view3d->pick(200, 200, model2);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model2);
    QCOMPARE(result.distance(), 600.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.0f, 0.0f));
    QCOMPARE(result.scenePosition(), QVector3D(0.0f, 0.0f, 0.0f));
    QCOMPARE(result.position(), QVector3D(-50.0f, -50.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // The bottom right of instancedModel,
    // overlapped model1 and then model2,
    // only check for instancedModel, should hit instancedModel
    result = view3d->pick(225, 175, instancedModel);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), instancedModel);
    QCOMPARE(result.distance(), 650.0f);
    QCOMPARE(result.uvPosition(), QVector2D(1.0f, 0.0f));
    QCOMPARE(result.scenePosition(), QVector3D(25.0f, 25.0f, -50.0f));
    QCOMPARE(result.position(), QVector3D(50.0f, -50.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 1);
    QCOMPARE(result.itemHit(), nullptr);

    // Pick subset based on viewport position

    // Upper right corner of model1, center of model2
    // Check for model2 only
    // Should hit model2, ignore model1
    QJSEngine engine;
    QJSValue array = engine.newArray(1);
    array.setProperty(0, engine.newQObject(model2));
    auto resultList = view3d->pickSubset(250, 150, array);
    QCOMPARE(resultList.size(), 1);
    QCOMPARE(resultList[0].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[0].objectHit(), model2);
    QCOMPARE(resultList[0].distance(), 600.0f);
    QCOMPARE(resultList[0].uvPosition(), QVector2D(0.5f, 0.5f));
    QCOMPARE(resultList[0].scenePosition(), QVector3D(50.0f, 50.0f, 0.0f));
    QCOMPARE(resultList[0].position(), QVector3D(0.0f, 0.0f, 50.0f));
    QCOMPARE(resultList[0].sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(resultList[0].normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(resultList[0].instanceIndex(), 0);
    QCOMPARE(resultList[0].itemHit(), nullptr);

    // The bottom right of instancedModel,
    // overlapped model1 and then model2,
    // Check for both model2 and instancedModel
    // Should hit both and ignore model1
    array = engine.newArray(2);
    array.setProperty(0, engine.newQObject(model2));
    array.setProperty(1, engine.newQObject(instancedModel));
    resultList = view3d->pickSubset(225, 175, array);
    QCOMPARE(resultList.size(), 2);
    QCOMPARE(resultList[0].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[0].objectHit(), model2);
    QCOMPARE(resultList[0].distance(), 600.0f);
    QCOMPARE(resultList[0].uvPosition(), QVector2D(0.25f, 0.25f));
    QCOMPARE(resultList[0].scenePosition(), QVector3D(25.0f, 25.0f, 0.0f));
    QCOMPARE(resultList[0].position(), QVector3D(-25.0f, -25.0f, 50.0f));
    QCOMPARE(resultList[0].sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(resultList[0].normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(resultList[0].instanceIndex(), 0);
    QCOMPARE(resultList[0].itemHit(), nullptr);
    QCOMPARE(resultList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[1].objectHit(), instancedModel);
    QCOMPARE(resultList[1].distance(), 650.0f);
    QCOMPARE(resultList[1].uvPosition(), QVector2D(1.0f, 0.0f));
    QCOMPARE(resultList[1].scenePosition(), QVector3D(25.0f, 25.0f, -50.0f));
    QCOMPARE(resultList[1].position(), QVector3D(50.0f, -50.0f, 50.0f));
    QCOMPARE(resultList[1].sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(resultList[1].normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(resultList[1].instanceIndex(), 1);
    QCOMPARE(resultList[1].itemHit(), nullptr);


    // Enable the 2D item
    item2d->setEnabled(true);
    QTest::qWait(100);
    // Then picking on top of model1 should not pick it anymore
    result = view3d->pick(200, 200);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2d);
    QCOMPARE(result.distance(), 400.0f);
    QCOMPARE(result.uvPosition(), QVector2D(75.0f, 75.0f));
    QCOMPARE(result.scenePosition(), QVector3D(0.0f, 0.0f, 200.0f));
    QCOMPARE(result.position(), QVector3D(0.0f, 0.0f, 0.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), -1);
    QCOMPARE(result.objectHit(), nullptr);
    // Hide the 2D item
    item2d->setVisible(false);
    QTest::qWait(100);
    // Then picking on top of model1 should pick it again
    result = view3d->pick(200, 200);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model1);

    // Pick all based on viewport position

    // Center of model1, bottom left corner of model2
    resultList = view3d->pickAll(200, 200);
    QCOMPARE(resultList.size(), 2);
    QCOMPARE(resultList[0].objectHit(), model1);
    QCOMPARE(resultList[1].objectHit(), model2);

    // Add the 2D item
    item2d->setVisible(true);
    QTest::qWait(100);
    resultList = view3d->pickAll(200, 200);
    QCOMPARE(resultList.size(), 3);
    QCOMPARE(resultList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultList[0].itemHit(), item2d);
    QCOMPARE(resultList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[1].objectHit(), model1);
    QCOMPARE(resultList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[2].objectHit(), model2);
    // Hide the 2D item
    item2d->setVisible(false);
    QTest::qWait(100);

    // Top right corner of model1, center of model2
    resultList = view3d->pickAll(250, 150);
    QCOMPARE(resultList.size(), 2);
    QCOMPARE(resultList[0].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[0].objectHit(), model1);
    QCOMPARE(resultList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[1].objectHit(), model2);

    // Just outside model1's upper right corner, so should hit the model behind (model2)
    resultList = view3d->pickAll(251, 151);
    QCOMPARE(resultList.size(), 1);
    QCOMPARE(resultList[0].objectHit(), model2);

    // The bottom right of the second entry in the instance table,
    // overlapping model1 and model2
    resultList = view3d->pickAll(225, 175);
    QCOMPARE(resultList.size(), 3);
    QCOMPARE(resultList[0].objectHit(), model1);
    QCOMPARE(resultList[1].objectHit(), model2);
    QCOMPARE(resultList[2].objectHit(), instancedModel);
    QCOMPARE(resultList[2].instanceIndex(), 1);

    // Just outside model2's upper right corner, so there should be no hit
    resultList = view3d->pickAll(301, 99);
    QVERIFY(resultList.isEmpty());
}

void tst_Picking::test_ray_picking()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("picking.qml"), QSize(400, 400)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *model1 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model1"));
    QVERIFY(model1);
    QQuick3DModel *model2 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model2"));
    QVERIFY(model2);
    QQuick3DModel *instancedModel = view3d->findChild<QQuick3DModel *>(QStringLiteral("instancedModel"));
    QVERIFY(instancedModel);
    QQuickItem *item2d = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dNode", "item2d"));
    QVERIFY(item2d);

    item2d->setEnabled(true);
    item2d->setVisible(true);
    QTest::qWait(100);

    // Ray based picking one result

    // Down the z axis from 0,0,100 (towards 0,0,0)
    QVector3D origin(0.0f, 0.0f, 100.0f);
    QVector3D direction(0.0f, 0.0f, -1.0f);
    auto result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model1);
    QCOMPARE(result.distance(), 50.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.5f, 0.5f));
    QCOMPARE(result.scenePosition(), QVector3D(0.0f, 0.0f, 50.0f));
    QCOMPARE(result.position(), QVector3D(0.0f, 0.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Down the z axis from 0,0,250 (towards 0,0,0) through the item2d
    origin = QVector3D(0.0f, 0.0f, 250.0f);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2d);
    QCOMPARE(result.distance(), 50.0f);
    QCOMPARE(result.uvPosition(), QVector2D(75.0f, 75.0f));
    QCOMPARE(result.scenePosition(), QVector3D(0.0f, 0.0f, 200.0f));
    QCOMPARE(result.position(), QVector3D(0.0f, 0.0f, 0.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), -1);
    QCOMPARE(result.objectHit(), nullptr);

    // Up the z axis from 0, 0, -101 (towards 0,0,0)
    origin = QVector3D(0.0f, 0.0f, -101.0f);
    direction = QVector3D(0.0f, 0.0f, 1.0f);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model2);
    QCOMPARE(result.distance(), 1.0f);
    QCOMPARE(result.uvPosition(), QVector2D(1.0f, 0.0f));
    QCOMPARE(result.scenePosition(), QVector3D(0.0f, 0.0f, -100.0f));
    QCOMPARE(result.position(), QVector3D(-50.0f, -50.0f, -50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, -1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, -1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Up the z axis from 0, 0, -100 (towards 0,0,-1000)
    origin = QVector3D(0.0f, 0.0f, -100.0f);
    direction = QVector3D(0.0f, 0.0f, -1.0f);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QCOMPARE(result.objectHit(), nullptr);
    QCOMPARE(result.itemHit(), nullptr);

    // Ray based picking all results

    // Down the z axis from 0,0,250 (towards 0,0,0)
    origin = QVector3D(0.0f, 0.0f, 250.0f);
    direction = QVector3D(0.0f, 0.0f, -1.0f);
    auto resultList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultList.size(), 3);
    QCOMPARE(resultList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultList[0].itemHit(), item2d);
    QCOMPARE(resultList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[1].objectHit(), model1);
    QCOMPARE(resultList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[2].objectHit(), model2);
    QVector3D hitPoint = resultList[2].scenePosition();

    // Test ray picking only model 2
    result = view3d->rayPick(origin, direction, model2);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model2);
    QCOMPARE(result.scenePosition(), hitPoint);

    // Up the z axis from 0, 0, -101 (towards 0,0,0)
    origin = QVector3D(0.0f, 0.0f, -101.0f);
    direction = QVector3D(0.0f, 0.0f, 1.0f);
    resultList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultList.size(), 2);
    QCOMPARE(resultList[0].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[0].objectHit(), model2);
    QCOMPARE(resultList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultList[1].objectHit(), model1);

    // Up the z axis from 0, 0, -100 (towards 0,0,-1000)
    origin = QVector3D(0.0f, 0.0f, -100.0f);
    direction = QVector3D(0.0f, 0.0f, -1.0f);
    resultList = view3d->rayPickAll(origin, direction);
    QVERIFY(resultList.isEmpty());
}


void tst_Picking::test_object_picking2()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("picking2.qml"), QSize(100, 100)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    const qreal dpr = view->devicePixelRatio();
    if (dpr != 1.0) {
        QSKIP("Test uses window positions to get exact values and those assume DPR of 1.0");
    }

    const auto viewSize = view->size();
    const float halfWidth = viewSize.width() * 0.5f;
    const float halfHeight = viewSize.height() * 0.5f;
    const float horizontalPickLine = halfHeight - 35;

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *coneModel = view3d->findChild<QQuick3DModel *>(QStringLiteral("model3"));
    QVERIFY(coneModel);

    // just left of the cone (model3)
    auto result = view3d->pick(halfWidth - 11, horizontalPickLine);
    QCOMPARE(result.objectHit(), nullptr);

    // center top of the cone (model3)
    result = view3d->pick(halfWidth, horizontalPickLine);
    QVERIFY(result.objectHit() != nullptr);
    QCOMPARE(result.objectHit(), coneModel);

    // just right of the cone (model3)
    result = view3d->pick(halfWidth + 11, horizontalPickLine);
    QCOMPARE(result.objectHit(), nullptr);
}

// Necessary for doing comparisons of normalized vectors
static bool fuzzyCompareVectors(const QVector3D& v1, const QVector3D& v2, float epsilon = 0.000001f) {
    return std::abs(v1.x() - v2.x()) < epsilon &&
           std::abs(v1.y() - v2.y()) < epsilon &&
           std::abs(v1.z() - v2.z()) < epsilon;

}

void tst_Picking::test_item_picking()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("item_picking.qml"), QSize(400, 400)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *model1 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model1"));
    QVERIFY(model1);
    QQuick3DModel *model2 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model2"));
    QVERIFY(model2);
    QQuickItem *item2dPlusZ = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dPlusZNode", "item2dPlusZ"));
    QVERIFY(item2dPlusZ);
    QQuickItem *item2dMinusZ = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dMinusZNode", "item2dMinusZ"));
    QVERIFY(item2dMinusZ);
    QQuickItem *item2dPlusX = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dPlusXNode", "item2dPlusX"));
    QVERIFY(item2dPlusX);
    QQuickItem *item2dMinusX = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dMinusXNode", "item2dMinusX"));
    QVERIFY(item2dMinusX);
    QQuickItem *item2dPlusY = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dPlusYNode", "item2dPlusY"));
    QVERIFY(item2dPlusY);
    QQuickItem *item2dMinusY = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dMinusYNode", "item2dMinusY"));
    QVERIFY(item2dMinusY);

    QQuickItem *item2dRotation = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dRotationNode", "item2dRotation"));
    QVERIFY(item2dRotation);
    QQuickItem *item2dTranslation = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dTranslationNode", "item2dTranslation"));
    QVERIFY(item2dTranslation);
    QQuickItem *item2dScale = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dScaleNode", "item2dScale"));
    QVERIFY(item2dScale);
    QQuickItem *item2dComplex = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dComplexNode", "item2dComplex"));
    QVERIFY(item2dComplex);
    QQuickItem *item2dMadness = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dMadnessNode", "item2dMadness"));
    QVERIFY(item2dMadness);


    // These tests intentionally try to cast rays into nothing, to check that item2ds picking stays within the bounds of the items
    // This is necessary since items2ds uses plane intersection for ray collision tests
    // Z Axes
    auto origin = QVector3D(200, 200, 200);
    auto direction = QVector3D(0, 0, -1);
    auto result = view3d->rayPick(origin, direction);
    auto resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    origin = QVector3D(200, 200, -200);
    direction = QVector3D(0, 0, -1);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // X Axes
    origin = QVector3D(200, 200, 200);
    direction = QVector3D(-1, 0, 0);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    origin = QVector3D(-200, 200, 200);
    direction = QVector3D(1, 0, 0);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // Y Axes
    origin = QVector3D(200, 200, 200);
    direction = QVector3D(0, -1, 0);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    origin = QVector3D(200, -200, 200);
    direction = QVector3D(0, 1, 0);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // These tests all cast a ray down from above
    direction = QVector3D (0, -1, 0);
    // Rotation
    origin = QVector3D(250, 100, -50);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // Translation
    origin = QVector3D(450, 100, 10);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // Scale
    origin = QVector3D(700, 100, 110);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // Complex
    origin = QVector3D(900, 100, 110);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // Madness (fake 3D transforms in 2D)
    origin = QVector3D(1150, 100, 90);
    result = view3d->rayPick(origin, direction);
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);
    QVERIFY(resultsList.isEmpty());

    // The following tests try to hit the item2ds
    // NB: Only a front-face hit of an item2d should count as a hit

    // +Z
    origin = QVector3D(25, 25, 200);
    direction = QVector3D(0, 0, -1);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dPlusZ);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 0, 1)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 3);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dPlusZ);
    QCOMPARE(resultsList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[1].objectHit(), model1);
    QCOMPARE(resultsList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[2].objectHit(), model2);

    // -Z
    // NB: In this case the distance of collision should be the exact same for both
    // model2 and item2dMinusZ. Item2D collision always occurs first, so it should
    // be the first in the list.
    origin = QVector3D(25, 25, -200);
    direction = QVector3D(0, 0, 1);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dMinusZ);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 0, -1)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 3);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dMinusZ);
    QCOMPARE(resultsList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[1].objectHit(), model2);
    QCOMPARE(resultsList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[2].objectHit(), model1);
    QCOMPARE(resultsList[0].distance(), resultsList[1].distance());

    // +X
    // NB: in this case the distance of collision should be the exact same for both
    // model2 and item2dPlusX. Item2D collision always occurs first, so it should
    // be the first in the list.
    origin = QVector3D(200, 25, -25);
    direction = QVector3D(-1, 0, 0);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dPlusX);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(1, 0, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 3);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dPlusX);
    QCOMPARE(resultsList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[1].objectHit(), model2);
    QCOMPARE(resultsList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[2].objectHit(), model1);
    QCOMPARE(resultsList[0].distance(), resultsList[1].distance());

    // -X
    origin = QVector3D(-200, 25, -25);
    direction = QVector3D(1, 0, 0);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dMinusX);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(-1, 0, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 3);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dMinusX);
    QCOMPARE(resultsList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[1].objectHit(), model1);
    QCOMPARE(resultsList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[2].objectHit(), model2);

    // +Y
    // NB: in this case the distance of collision should be the exact same for both
    // model2 and item2dPlusY. Item2D collision always occurs first, so it should
    // be the first in the list.
    origin = QVector3D(25, 200, -25);
    direction = QVector3D(0, -1, 0);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dPlusY);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 1, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 3);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dPlusY);
    QCOMPARE(resultsList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[1].objectHit(), model2);
    QCOMPARE(resultsList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[2].objectHit(), model1);
    QCOMPARE(resultsList[0].distance(), resultsList[1].distance());

    // -Y
    origin = QVector3D(25, -200, -25);
    direction = QVector3D(0, 1, 0);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dMinusY);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, -1, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 3);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dMinusY);
    QCOMPARE(resultsList[1].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[1].objectHit(), model1);
    QCOMPARE(resultsList[2].hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(resultsList[2].objectHit(), model2);


    // The transform tests are aligned to the XZ plane, so all directions
    // will be down the Y axis
    direction = QVector3D(0, -1, 0);

    // Rotation
    origin = QVector3D(300, 100, 0);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dRotation);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 1, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 1);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dRotation);

    // Translate
    origin = QVector3D(450, 100, -50);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dTranslation);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 1, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 1);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dTranslation);

    // Scale
    origin = QVector3D(700, 100, 0);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dScale);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 1, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 1);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dScale);

    // Complex
    origin = QVector3D(900, 100, 0);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dComplex);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 1, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 1);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dComplex);

    // Madness
    origin = QVector3D(1150, 100, 50);
    result = view3d->rayPick(origin, direction);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(result.itemHit(), item2dMadness);
    QVERIFY(fuzzyCompareVectors(result.sceneNormal(), QVector3D(0, 1, 0)));
    resultsList = view3d->rayPickAll(origin, direction);
    QCOMPARE(resultsList.size(), 1);
    QCOMPARE(resultsList[0].hitType(), QQuick3DPickResultEnums::HitType::Item);
    QCOMPARE(resultsList[0].itemHit(), item2dMadness);

}


// When triangles become so small the picking algorithm fails because of floating point
// precision. This test generates a quad with count * count subdivisions leading to smaller
// and smaller triangles to pick against. Even if a mesh is very dense, the triangle intersection
// code should still work and return model1 as a hit.
void tst_Picking::test_picking_QTBUG_111997()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("qtbug_111997.qml"), QSize(100, 100)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *model1 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model1"));
    QVERIFY(model1);

    const auto viewSize = view->size();
    const float halfWidth = viewSize.width() * 0.5f;
    const float halfHeight = viewSize.height() * 0.5f;
    QSignalSpy frameSwappedSpy(view.data(), &QQuickWindow::frameSwapped);

    // Add custom geometry to the scene
    QScopedPointer<CustomGeometry> geometry(new CustomGeometry(20));
    geometry->setParent(model1);
    geometry->setParentItem(model1);
    model1->setGeometry(geometry.data());

    // Wait for the next frame (so that geometry is up-to-date
    if (!frameSwappedSpy.wait())
        QFAIL("frameSwapped() signal not emitted");

    // Check picking with 20 subdivisions
    auto result = view3d->pick(halfWidth, halfHeight);
    QVERIFY(result.objectHit() != nullptr);
    QCOMPARE(result.objectHit(), model1);

    // check picking with 350 subdivisions
    geometry->setCount(350);
    if (!frameSwappedSpy.wait())
        QFAIL("frameSwapped() signal not emitted");
    result = view3d->pick(halfWidth, halfHeight);
    QVERIFY(result.objectHit() != nullptr);
    QCOMPARE(result.objectHit(), model1);

    // check picking with 500 subdivisions
    geometry->setCount(500);
    if (!frameSwappedSpy.wait())
        QFAIL("frameSwapped() signal not emitted");
    result = view3d->pick(halfWidth, halfHeight);
    QVERIFY(result.objectHit() != nullptr);
    QCOMPARE(result.objectHit(), model1);

}

void tst_Picking::test_sphere_geometry()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("pickable_sphere.qml"), QSize(100, 100)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *model1 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model1"));
    QVERIFY(model1);

    const auto viewSize = view->size();
    const float halfWidth = viewSize.width() * 0.5f;
    const float halfHeight = viewSize.height() * 0.5f;
    QSignalSpy frameSwappedSpy(view.data(), &QQuickWindow::frameSwapped);

    // Wait for the next frame (so that geometry is up-to-date
    if (!frameSwappedSpy.wait())
        QFAIL("frameSwapped() signal not emitted");

    auto result = view3d->pick(halfWidth, halfHeight);
    QVERIFY(result.objectHit() != nullptr);
    QCOMPARE(result.objectHit(), model1);
}

void tst_Picking::test_picking_corner_case()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("corner_case.qml"), QSize(400, 400)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *model1 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model1"));
    QVERIFY(model1);

    const QVector3D direction = QVector3D(0.0f, 0.0f, 1.0f);

    {
        // bottom right corner (vertex)
        const QVector3D origin1 = QVector3D(0.0f, 0.0f, -101.0f);
        const QVector3D origin2 = QVector3D(0.0f, 0.0f, -100.0f);
        auto result = view3d->rayPick(origin1, direction);
        QVERIFY(result.objectHit() != nullptr);
        QCOMPARE(result.objectHit(), model1);
        QCOMPARE(result.scenePosition(), origin2);
        QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, -1.0f));
        QCOMPARE(result.uvPosition(), QVector2D(1.0f, 0.0f));

        // The ray doesn't intersect the triangle as the origin
        // starts on the triangle

        // The ray intersects with the other face of the cube,
        // but unless we test for backfaces the test should still

        result = view3d->rayPick(origin2, direction);
        QVERIFY(result.objectHit() == nullptr);
    }

    {
        // top right corner (vertex)
        const QVector3D origin1 = QVector3D(0.0f, 100.0f, -101.0f);
        const QVector3D origin2 = QVector3D(0.0f, 100.0f, -100.0f);
        auto result = view3d->rayPick(origin1, direction);
        QVERIFY(result.objectHit() != nullptr);
        QCOMPARE(result.objectHit(), model1);
        QCOMPARE(result.scenePosition(), origin2);
        QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, -1.0f));
        QCOMPARE(result.uvPosition(), QVector2D(1.0f, 1.0f));

        // The ray doesn't intersect the triangle as the origin
        // starts on the triangle

        // The ray intersects with the other face of the cube,
        // but unless we test for backfaces the test should still

        result = view3d->rayPick(origin2, direction);
        QVERIFY(result.objectHit() == nullptr);
    }

    {
        // bottom left corner (vertex)
        const QVector3D origin1 = QVector3D(100.0f, 0.0f, -101.0f);
        const QVector3D origin2 = QVector3D(100.0f, 0.0f, -100.0f);
        auto result = view3d->rayPick(origin1, direction);
        QVERIFY(result.objectHit() != nullptr);
        QCOMPARE(result.objectHit(), model1);
        QCOMPARE(result.scenePosition(), origin2);
        QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, -1.0f));
        QCOMPARE(result.uvPosition(), QVector2D(0.0f, 0.0f));

        // The ray doesn't intersect the triangle as the origin
        // starts on the triangle

        // The ray intersects with the other face of the cube,
        // but unless we test for backfaces the test should still

        result = view3d->rayPick(origin2, direction);
        QVERIFY(result.objectHit() == nullptr);
    }

    {
        // top left corner (vertex)
        const QVector3D origin1 = QVector3D(100.0f, 100.0f, -101.0f);
        const QVector3D origin2 = QVector3D(100.0f, 100.0f, -100.0f);
        auto result = view3d->rayPick(origin1, direction);
        QVERIFY(result.objectHit() != nullptr);
        QCOMPARE(result.objectHit(), model1);
        QCOMPARE(result.scenePosition(), origin2);
        QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, -1.0f));
        QCOMPARE(result.uvPosition(), QVector2D(0.0f, 1.0f));

        // The ray doesn't intersect the triangle as the origin
        // starts on the triangle

        // The ray intersects with the other face of the cube,
        // but unless we test for backfaces the test should still

        result = view3d->rayPick(origin2, direction);
        QVERIFY(result.objectHit() == nullptr);
    }

    {
        // Center
        const QVector3D origin1 = QVector3D(50.0f, 50.0f, -101.0f);
        const QVector3D origin2 = QVector3D(50.0f, 50.0f, -100.0f);
        auto result = view3d->rayPick(origin1, direction);
        QVERIFY(result.objectHit() != nullptr);
        QCOMPARE(result.objectHit(), model1);
        QCOMPARE(result.scenePosition(), origin2);
        QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, -1.0f));
        QCOMPARE(result.uvPosition(), QVector2D(0.5f, 0.5f));

        // The ray doesn't intersect the triangle as the origin
        // starts on the triangle

        // The ray intersects with the other face of the cube,
        // but unless we test for backfaces the test should still

        result = view3d->rayPick(origin2, direction);
        QVERIFY(result.objectHit() == nullptr);
    }

}

void tst_Picking::test_triangleIntersect()
{
    const QVector3D direction(0, 0, -1);
    const QVector3D expectedNormal(0, 0, 1);
    const QVector3D vert0(0, 0, 0);
    const QVector3D vert1(-1, -1, 0);
    const QVector3D vert2(1, -1, 0);

    QVector3D normal;
    float u;
    float v;
    bool intersects;
    const QVector3D origin1(0, 0, 2);

    const QSSGRenderRay ray1(origin1, direction); // intersection vert0

    intersects = QSSGRenderRay::triangleIntersect(ray1,
                                                  vert0,
                                                  vert1,
                                                  vert2,
                                                  u,
                                                  v,
                                                  normal);
    QVERIFY(intersects);
    QCOMPARE(normal, expectedNormal);
    QCOMPARE(u, 0.0f);
    QCOMPARE(v, 0.0f);

    const QVector3D origin2(-1, -1, 2);
    const QSSGRenderRay ray2(origin2, direction); // intersection vert1

    intersects = QSSGRenderRay::triangleIntersect(ray2,
                                                  vert0,
                                                  vert1,
                                                  vert2,
                                                  u,
                                                  v,
                                                  normal);
    QVERIFY(intersects);
    QCOMPARE(normal, expectedNormal);
    QCOMPARE(u, 1.0f);
    QCOMPARE(v, 0.0f);

    const QVector3D origin3(1, -1, 2);
    const QSSGRenderRay ray3(origin3, direction); // intersection vert2
    intersects = QSSGRenderRay::triangleIntersect(ray3,
                                                  vert0,
                                                  vert1,
                                                  vert2,
                                                  u,
                                                  v,
                                                  normal);
    QVERIFY(intersects);
    QCOMPARE(normal, expectedNormal);
    QCOMPARE(u, 0.0f);
    QCOMPARE(v, 1.0f);
}

void tst_Picking::test_closest_point_picking()
{
    QScopedPointer<QQuickView> view(createView(QLatin1String("picking.qml"), QSize(400, 400)));
    QVERIFY(view);
    QVERIFY(QTest::qWaitForWindowExposed(view.data()));

    QQuick3DViewport *view3d = view->findChild<QQuick3DViewport *>(QStringLiteral("view"));
    QVERIFY(view3d);
    QQuick3DModel *model1 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model1"));
    QVERIFY(model1);
    QQuick3DModel *model2 = view3d->findChild<QQuick3DModel *>(QStringLiteral("model2"));
    QVERIFY(model2);
    QQuick3DModel *instancedModel = view3d->findChild<QQuick3DModel *>(QStringLiteral("instancedModel"));
    QVERIFY(instancedModel);
    QQuickItem *item2d = qmlobject_cast<QQuickItem *>(find2DChildIn3DNode(view.data(), "item2dNode", "item2d"));
    QVERIFY(item2d);

    item2d->setEnabled(true);
    item2d->setVisible(true);
    QTest::qWait(100);

    // model1 is a unit cube at (0,0,0), so it has a surface point at (0, 0, 50)

    QVector3D origin;

    // Model outside radius
    origin = QVector3D(0.0f, 0.0f, 70.0f);
    auto result = view3d->closestPointPick(origin, 10);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Null);

    result = view3d->closestPointPick(origin, 30);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model1);
    QCOMPARE(result.distance(), 20.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.5f, 0.5f));
    QCOMPARE(result.scenePosition(), QVector3D(0.0f, 0.0f, 50.0f));
    QCOMPARE(result.position(), QVector3D(0.0f, 0.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Test hit on edge. It's not specified which face it should choose, so ignore
    // normals and UVs. model1 has an edge that passes through (-50, -50, 0)

    origin = QVector3D(-70.0f, -70.0f, 0.0f); // distance == sqrt(20^2 + 20^2) < 30
    result = view3d->closestPointPick(origin, 30, model1);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model1);
    QCOMPARE(result.scenePosition(), QVector3D(-50.0f, -50.0f, 0.0f));
    QVERIFY(qFuzzyCompare(result.distance(), float(sqrt(800.0f))));

    // model2 is a unit cube at (50, 50, -50), so it has a surface point at (10, 10, 0)
    // Check to see that we return the specified model, even if model1 is closer
    origin = QVector3D(10.0f, 10.0f, 70.0f);
    result = view3d->closestPointPick(origin, 100, model2);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), model2);
    QCOMPARE(result.distance(), 70.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.1f, 0.1f));
    QCOMPARE(result.scenePosition(), QVector3D(10.0f, 10.0f, 0.0f));
    QCOMPARE(result.position(), QVector3D(-40.0f, -40.0f, 50.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.normal(), QVector3D(0.0f, 0.0f, 1.0f));
    QCOMPARE(result.instanceIndex(), 0);
    QCOMPARE(result.itemHit(), nullptr);

    // Test instanced picking
    // Instance index 2 describes a unit cube at (200, 0, 0), so it has a surface
    // point at (150, 0, 0) with a normal of (-1, 0, 0)
    origin = QVector3D(140.0f, 0.0f, 0.0f);
    result = view3d->closestPointPick(origin, 20);
    QCOMPARE(result.hitType(), QQuick3DPickResultEnums::HitType::Model);
    QCOMPARE(result.objectHit(), instancedModel);
    QCOMPARE(result.distance(), 10.0f);
    QCOMPARE(result.uvPosition(), QVector2D(0.5f, 0.5f));
    QCOMPARE(result.scenePosition(), QVector3D(150.0f, 0.0f, 0.0f));
    QCOMPARE(result.position(), QVector3D(-50.0f, 0.0f, 0.0f));
    QCOMPARE(result.sceneNormal(), QVector3D(-1.0f, 0.0f, 0.0f));
    QCOMPARE(result.normal(), QVector3D(-1.0f, 0.0f, 0.0f));
    QCOMPARE(result.instanceIndex(), 2);
    QCOMPARE(result.itemHit(), nullptr);
}

QTEST_MAIN(tst_Picking)
#include "tst_picking.moc"

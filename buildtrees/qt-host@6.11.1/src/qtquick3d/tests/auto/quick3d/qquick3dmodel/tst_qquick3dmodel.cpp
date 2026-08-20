// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>

#include <QtQuick3D/private/qquick3dmodel_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>

class tst_QQuick3DModel : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class Model : public QQuick3DModel
    {
    public:
        using QQuick3DModel::updateSpatialNode;
    };

private slots:
    void testProperties();
    void testEnums();
};

void tst_QQuick3DModel::testProperties()
{
    Model model;
    auto node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    model.setCastsShadows(true);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(model.castsShadows());
    QVERIFY(node->castsShadows);
    model.setCastsShadows(false);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(!model.castsShadows());
    QVERIFY(!node->castsShadows);

    model.setReceivesShadows(true);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(model.receivesShadows());
    QVERIFY(node->receivesShadows);
    model.setReceivesShadows(false);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(!model.receivesShadows());
    QVERIFY(!node->receivesShadows);

    model.setReceivesReflections(true);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(model.receivesReflections());
    QVERIFY(node->receivesReflections);
    model.setReceivesReflections(false);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(!model.receivesReflections());
    QVERIFY(!node->receivesReflections);

    model.setPickable(true);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(model.pickable());
    QVERIFY(node->getLocalState(QSSGRenderModel::LocalState::Pickable));
    model.setPickable(false);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(!model.receivesShadows());
    QVERIFY(!node->getLocalState(QSSGRenderModel::LocalState::Pickable));

    model.setLevelOfDetailBias(0.0f);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(model.levelOfDetailBias() == 0.0f);
    QVERIFY(node->levelOfDetailBias == 0.0f);
    model.setLevelOfDetailBias(1.0f);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QVERIFY(model.levelOfDetailBias() == 1.0f);
    QVERIFY(node->levelOfDetailBias == 1.0f);

    // mesh from source
    QUrl cubeUrl("#Cube");
    QSignalSpy spy(&model, SIGNAL(sourceChanged()));
    model.setSource(cubeUrl);
    QCOMPARE(spy.size(), 1);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QCOMPARE(cubeUrl, model.source());
    QCOMPARE(cubeUrl, QUrl{node->meshPath.path()});
    QCOMPARE(originalNode, node);

    QQuick3DGeometry geometry;
    model.setGeometry(&geometry);
    node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(node));
    QCOMPARE(&geometry, model.geometry());
}

void tst_QQuick3DModel::testEnums()
{
    Model model;
    auto node = static_cast<QSSGRenderModel *>(model.updateSpatialNode(nullptr));
    QVERIFY(node);
}

QTEST_APPLESS_MAIN(tst_QQuick3DModel)
#include "tst_qquick3dmodel.moc"

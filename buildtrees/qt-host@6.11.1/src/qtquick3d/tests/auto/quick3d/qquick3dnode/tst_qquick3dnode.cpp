// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>

#include <QtGui/qquaternion.h>

class tst_QQuick3DNode : public QObject
{
    Q_OBJECT

    // Work-around to get access to updateSpatialNode
    class NodeItem : public QQuick3DNode
    {
    public:
        using QQuick3DNode::updateSpatialNode;
    };

private slots:
    void testProperties();
    void testSceneRotation();
    void testEnums();
    void testPositionMapping();
    void testDirectionMapping();
};

void tst_QQuick3DNode::testProperties()
{
    NodeItem nodeItem;
    auto node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(nullptr));
    const auto originalNode = node; // for comparisons later...
    QVERIFY(node);

    // Positions
    const float x = 50.0f;
    nodeItem.setX(x);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QCOMPARE(x, nodeItem.x());
    QVERIFY(qFuzzyCompare(x, QSSGUtils::mat44::getPosition(node->localTransform).x()));

    const float y = 60.0f;
    nodeItem.setY(y);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QCOMPARE(y, nodeItem.y());
    QVERIFY(qFuzzyCompare(y, QSSGUtils::mat44::getPosition(node->localTransform).y()));

    const float z = 70.0f;
    nodeItem.setZ(z);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QCOMPARE(z, nodeItem.z());
    QVERIFY(qFuzzyCompare(z, QSSGUtils::mat44::getPosition(node->localTransform).z()));

    QVector3D pos1(x, y, z);
    QCOMPARE(pos1, nodeItem.position());
    QVERIFY(qFuzzyCompare(pos1, QSSGUtils::mat44::getPosition(node->localTransform)));

    pos1 *= 2.0;
    nodeItem.setPosition(pos1);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QCOMPARE(pos1, nodeItem.position());
    QVERIFY(qFuzzyCompare(pos1, QSSGUtils::mat44::getPosition(node->localTransform)));

    // Rotation, Scale, Pivot
    QCOMPARE(nodeItem.rotation(), QQuaternion());
    QCOMPARE(nodeItem.scale(), QVector3D(1, 1, 1));
    QCOMPARE(nodeItem.pivot(), QVector3D());

    QQuaternion rot = QQuaternion::fromEulerAngles(100, 200, 300);
    nodeItem.setRotation(rot);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QVERIFY(qFuzzyCompare(rot, nodeItem.rotation()));
    QVERIFY(qFuzzyCompare(rot, QQuaternion::fromRotationMatrix(QSSGUtils::mat44::getUpper3x3(node->localTransform))));

    QVector3D scale(0.5, 1.0, 2.0);
    nodeItem.setScale(scale);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QCOMPARE(scale, nodeItem.scale());
    QVERIFY(qFuzzyCompare(scale, QSSGUtils::mat44::getScale(node->localTransform)));

    QVector3D pivot(10, 20, 30);
    nodeItem.setPivot(pivot);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QCOMPARE(pivot, nodeItem.pivot());
    QCOMPARE(pivot, node->pivot);

    // Opacity
    const float opacity = 0.2f;
    nodeItem.setLocalOpacity(opacity);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QCOMPARE(opacity, nodeItem.localOpacity());
    QCOMPARE(opacity, node->localOpacity);

    // Visibility
    nodeItem.setVisible(false);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QVERIFY(!node->getLocalState(QSSGRenderNode::LocalState::Active));
    QSignalSpy spy(&nodeItem, SIGNAL(visibleChanged()));
    nodeItem.setVisible(true);
    node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(node));
    QVERIFY(node->getLocalState(QSSGRenderNode::LocalState::Active));
    QCOMPARE(spy.size(), 1);

    QCOMPARE(originalNode, node);
}

void tst_QQuick3DNode::testSceneRotation()
{
    QQuick3DNode parentNode;
    QQuick3DNode selfNode(&parentNode);

    const auto rotX45 = QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f);
    parentNode.setRotation(rotX45);
    QVERIFY(qFuzzyCompare(selfNode.sceneRotation(), rotX45));

    const auto rotZ30 = QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 0.0f, 1.0f), 30.0f);
    selfNode.setRotation(rotZ30);
    QVERIFY(qFuzzyCompare(selfNode.sceneRotation(), rotX45 * rotZ30));

    // !m_hasInheritedUniformScale path, but the XY plane is scaled uniformly.
    parentNode.setScale(QVector3D(5.0f, 5.0f, 1.0f));
    QVERIFY(qFuzzyCompare(selfNode.sceneRotation(), rotX45 * rotZ30));
}

void tst_QQuick3DNode::testEnums()
{
    NodeItem nodeItem;
    auto node = static_cast<QSSGRenderNode *>(nodeItem.updateSpatialNode(nullptr));
    QVERIFY(node);
}

void tst_QQuick3DNode::testPositionMapping()
{
    QQuick3DNode nodeA, nodeB;
    nodeB.setPosition(QVector3D{2.0f, 2.0f, 2.0f});

    { // map pos TO scene A
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapPositionToScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map pos TO scene B
        const QVector3D expected { 3.0f, 3.0f, 3.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeB.mapPositionToScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map pos FROM scene A
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapPositionFromScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map pos FROM scene B
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 3.0f, 3.0f, 3.0f };
        const auto res = nodeB.mapPositionFromScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map node A's postion TO node B
        const QVector3D expected { -1.0f, -1.0f, -1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapPositionToNode(&nodeB, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map node A's postion to node 'null'
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapPositionToNode(nullptr, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map pos FROM node B's pos
        const QVector3D expected { 3.0f, 3.0f, 3.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapPositionFromNode(&nodeB, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map pos FROM null node
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapPositionFromNode(nullptr, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }
}

void tst_QQuick3DNode::testDirectionMapping()
{
    QQuick3DNode nodeA, nodeB;
    nodeB.setEulerRotation(QVector3D{0.0f, 0.0f, 90.0f});

    { // map dir TO scene A
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapDirectionToScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map dir TO scene B
        const QVector3D expected { -1.0f, 1.0f, 0.0f };
        const QVector3D input { 1.0f, 1.0f, 0.0f };
        const auto res = nodeB.mapDirectionToScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map dir FROM scene A
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapDirectionFromScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map dir FROM scene B
        const QVector3D expected { 1.0f, -1.0f, 0.0f };
        const QVector3D input { 1.0f, 1.0f, 0.0f };
        const auto res = nodeB.mapDirectionFromScene(input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map direction FROM node A's TO node B's space
        const QVector3D expected { 1.0f, -1.0f, 0.0f };
        const QVector3D input { 1.0f, 1.0f, 0.0f };
        const auto res = nodeA.mapDirectionToNode(&nodeB, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map direction FROM node A's TO null node
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapDirectionToNode(nullptr, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map direction FROM node B's pos
        const QVector3D expected { -1.0f, 1.0f, 0.0f };
        const QVector3D input { 1.0f, 1.0f, 0.0f };
        const auto res = nodeA.mapDirectionFromNode(&nodeB, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }

    { // map pos FROM null node
        const QVector3D expected { 1.0f, 1.0f, 1.0f };
        const QVector3D input { 1.0f, 1.0f, 1.0f };
        const auto res = nodeA.mapDirectionFromNode(nullptr, input);
        QVERIFY(qFuzzyCompare(res, expected));
    }
}

QTEST_APPLESS_MAIN(tst_QQuick3DNode)
#include "tst_qquick3dnode.moc"

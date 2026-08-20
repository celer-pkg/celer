// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderroot_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderdata_p.h>


class tst_NodeIndexing : public QObject
{
    Q_OBJECT

public:
    tst_NodeIndexing() = default;
    ~tst_NodeIndexing() = default;

private slots:
    void initTestCase();
    void testIndexingWithChildren();
    void testIndexingWithMultipleViews();
    void testIndexingWithImportScene();
    void testVersionWrapAround();

private:
    static void removeFromLayer(QSSGRenderLayer &layer, std::vector<QSSGRenderNode *> &nodes);

    static void buildBasicNodeHierarchy(QSSGRenderNode *parent, std::vector<QSSGRenderNode *> &outList);

    QSSGRenderRoot rootNode;

    // 10 nodes with 2 children each means we have a total of 30 nodes
    static constexpr int initialNodeCount = 10 + (10 * 2);
};

void tst_NodeIndexing::initTestCase()
{

}

void tst_NodeIndexing::testIndexingWithChildren()
{
    QSSGRenderLayer layer;

    layer.ref(&rootNode);
    rootNode.addChild(layer);

    std::vector<QSSGRenderNode *> nodes;

    QVERIFY(layer.children.isEmpty());
    QVERIFY(layer.rootNode == &rootNode);

    buildBasicNodeHierarchy(&layer, nodes);

    QVERIFY(!nodes.empty());

    rootNode.reindex();

    std::unordered_set<quint32> nodeIndices;
    for (auto *node : nodes) {
        QVERIFY(node->h.hasId());
        nodeIndices.insert(node->h.index());
    }

    QCOMPARE(nodeIndices.size(), initialNodeCount);

    removeFromLayer(layer, nodes);
    rootNode.removeChild(layer);
    qDeleteAll(nodes);
}

void tst_NodeIndexing::testIndexingWithMultipleViews()
{
    // Multiple views means we have more then one layer
    constexpr size_t LayerCount = 3;
    std::array<QSSGRenderLayer, LayerCount> layers;

    for (auto &layer : layers) {
        QVERIFY(layer.children.isEmpty());
        layer.ref(&rootNode);
    }

    std::array<std::vector<QSSGRenderNode *>, LayerCount> layerNodes;

    // Add nodes to the extra layers
    for (size_t i = 0; i < LayerCount; ++i) {
        buildBasicNodeHierarchy(&layers[i], layerNodes[i]);
        QVERIFY(!layerNodes[i].empty());
    }

    // Add the layers to the root node
    for (auto &layer : layers) {
        rootNode.addChild(layer);
        QVERIFY(layer.rootNode == &rootNode);
    }

    // Reindex the root node, which should index all nodes in all layers
    rootNode.reindex();

    std::set<quint32> nodeIndices;
    // Check that all nodes in the testNodes and layerNodes have unique indices
    for (const auto &nodes : layerNodes) {
        for (auto *node : nodes) {
            QVERIFY(node->h.hasId());
            nodeIndices.insert(node->h.index());
        }
    }

    // Check that we have unique indices for all nodes
    QCOMPARE(nodeIndices.size(), initialNodeCount * LayerCount);

    quint32 expectedIndex = 1; // Start from 1 because the first index is reserved for the root/layer node(s)
    for (auto begin = nodeIndices.begin(), end = nodeIndices.end(); begin != end; ++begin) {
        // Check that the indices are without gaps
        QCOMPARE(expectedIndex, *begin);
        ++expectedIndex;
    }

    // Now remove all nodes from their layers
    for (size_t i = 0; i < LayerCount; ++i) {
        removeFromLayer(layers[i], layerNodes[i]);
        QVERIFY(layers[i].children.isEmpty());
        rootNode.removeChild(layers[i]);
    }

    for (auto &nodes : layerNodes)
        qDeleteAll(nodes);
}

void tst_NodeIndexing::testIndexingWithImportScene()
{
    // Multiple views means we have more then one layer
    constexpr size_t LayerCount = 3;
    std::array<QSSGRenderLayer, LayerCount> layers;

    for (auto &layer : layers) {
        QVERIFY(layer.children.isEmpty());
        layer.ref(&rootNode);
    }

    // In this case we'll have two sets of nodes, one for the main layer
    // and one for the import scene layers.
    std::array<std::vector<QSSGRenderNode *>, LayerCount> layerNodes;

    auto &mainLayerNodes = layerNodes[0];
    auto &mainLayer = layers[0];

    std::unique_ptr<QSSGRenderNode> sceneRoot(new QSSGRenderNode(QSSGRenderNode::Type::Node));
    mainLayer.addChild(*sceneRoot);

    // Build the main layer with the initial nodes
    buildBasicNodeHierarchy(sceneRoot.get(), mainLayerNodes);

    for (size_t i = 1; i < LayerCount; ++i) {
        // For the other layers, we'll just copy the main layer nodes
        layerNodes[i] = mainLayerNodes;
        // Create a new layer for the import scene
        auto &currentLayer = layers[i];
        QVERIFY(currentLayer.children.isEmpty());
        // Replicate how we do import scenes
        currentLayer.children.m_head = currentLayer.children.m_tail = sceneRoot.get();

    }

    // Add the layers to the root node
    for (auto &layer : layers) {
        rootNode.addChild(layer);
        QVERIFY(layer.rootNode == &rootNode);
    }

    // Reindex the root node, which should index all nodes in all layers
    rootNode.reindex();

    std::set<quint32> nodeIndices;
    size_t insertedNodeCount = 0;
    // Check that all nodes in the testNodes and layerNodes have unique indices
    for (const auto &nodes : layerNodes) {
        for (auto *node : nodes) {
            QVERIFY(node->h.hasId());
            ++insertedNodeCount;
            nodeIndices.insert(node->h.index());
        }
    }

    // Need to add the scene root as well
    QVERIFY(sceneRoot && sceneRoot->h.hasId());
    ++insertedNodeCount;
    nodeIndices.insert(sceneRoot->h.index());

    // Check that we have unique indices for all nodes
    // The total number of nodes should be initialNodeCount * LayerCount
    // (It would be better to get the numbers from the reindex function,
    // but this is good enough for now).
    QCOMPARE(insertedNodeCount, (initialNodeCount * LayerCount) + 1 /* for the scene root node */);
    // But the indices should only be as many as the main layer nodes
    QCOMPARE(nodeIndices.size(), mainLayerNodes.size() + 1 /* for the scene root node */);
    QCOMPARE(nodeIndices.size(), initialNodeCount + 1 /* for the scene root node */);

    // Check that the indices are in the expected range
    quint32 expectedIndex = 1; // Start from 1 because the first index is reserved for the root/layer node(s)
    for (auto begin = nodeIndices.begin(), end = nodeIndices.end(); begin != end; ++begin) {
        // Check that the indices are without gaps
        QCOMPARE(expectedIndex, *begin);
        ++expectedIndex;
    }

    // Now remove all nodes from their layers
    removeFromLayer(mainLayer, mainLayerNodes);
    mainLayer.removeChild(*sceneRoot);
    QVERIFY(mainLayer.children.isEmpty());

    // Ensure that the scene root isn't referenced by any layer
    for (size_t i = 0; i < LayerCount; ++i) {
        auto &layer = layers[i];
        layer.children.clear(); // Clear the list (or the sceneRoot will be used after it's deleted)
        rootNode.removeChild(layer);
    }

    qDeleteAll(mainLayerNodes);
}

void tst_NodeIndexing::testVersionWrapAround()
{
    using VersionType = QSSGRenderNodeVersionType;

    QSSGRenderLayer layer;
    layer.ref(&rootNode);
    rootNode.addChild(layer);

    auto *node = new QSSGRenderNode(QSSGRenderNode::Type::Node);
    layer.addChild(*node);

    rootNode.reindex();
    QVERIFY(node->h.hasId());

    // Bypass the counter directly to just below the maximum, avoiding the need
    // to iterate through all 65535 values in the test.
    auto &gnd = *rootNode.globalNodeData();
    gnd.m_version = std::numeric_limits<VersionType>::max() - 1;

    // Reindex to reach the maximum version value.
    rootNode.reindex();
    QVERIFY(node->h.hasId());
    QCOMPARE(node->h.version(), std::numeric_limits<VersionType>::max());
    QCOMPARE(layer.lh.version(), std::numeric_limits<VersionType>::max());

    // One more reindex wraps max + 1 to 0; the guard skips 0 and bumps to 1.
    // Nodes and the layer handle must remain valid.
    rootNode.reindex();
    QVERIFY(node->h.hasId());
    QCOMPARE(node->h.version(), VersionType(1));
    QCOMPARE(layer.lh.version(), VersionType(1));

    layer.removeChild(*node);
    delete node;
    rootNode.removeChild(layer);
}

void tst_NodeIndexing::removeFromLayer(QSSGRenderLayer &layer, std::vector<QSSGRenderNode *> &nodes)
{
    for (auto *node : nodes) {
        QSSGRenderNode *parent = node->parent;
        if (parent)
            parent->removeChild(*node);
        else
            layer.removeChild(*node);
    }

    // Check that all nodes no longer have parents and
    // their node index is reset.
    for (auto *node : nodes) {
        QCOMPARE(node->parent, nullptr);
        QVERIFY(!node->h.hasId());
    }
}

void tst_NodeIndexing::buildBasicNodeHierarchy(QSSGRenderNode *parent, std::vector<QSSGRenderNode *> &outList)
{
    Q_ASSERT(parent);
    Q_ASSERT(outList.size() == 0);

    for (int i = 0; i < initialNodeCount; ++i) {
        auto *node = new QSSGRenderNode(QSSGRenderNode::Type::Node);
        outList.push_back(node);
    }

    // Add nodes to the second layer
    for (int i = 0; i < initialNodeCount; i += 3) {
        auto *node = outList[i];
        QVERIFY(!node->h.hasId());
        parent->addChild(*node);

        // Add two children to each node
        for (int j = 0; j < 2; ++j) {
            auto *childNode = outList[i + j + 1];
            QVERIFY(!childNode->h.hasId());
            node->addChild(*childNode);
        }
    }
}

QTEST_APPLESS_MAIN(tst_NodeIndexing)

#include "tst_nodeindexing.moc"

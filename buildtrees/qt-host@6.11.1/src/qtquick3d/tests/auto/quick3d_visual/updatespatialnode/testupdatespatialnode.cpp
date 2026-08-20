// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testupdatespatialnode.h"
#include <ssg/qssgrenderextensions.h>
#include <ssg/qssgrenderhelpers.h>
#include <QThread>
#include <QTest>

#include <QtQuick3D/private/qquick3dobject_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>

template <typename T>
static void tst_updateSpatialNode(QQuick3DObject *owner,
                                  TestData &testData,
                                  QSSGRenderGraphObject *&node)
{
    QVERIFY(owner != nullptr);

    // If updateSpatialNode is called there shall be a sceneManager set
    const auto sceneManager = QQuick3DObjectPrivate::get(owner)->sceneManager;
    Q_ASSERT(sceneManager);

    const auto &nodeMap = sceneManager->m_nodeMap;

    if (testData.callCount == 0) {
        // 1. The first time we'll create a new node
        // There should be no links to this object in the node map
        const auto values = nodeMap.values();
        QVERIFY(!values.contains(owner));
        QVERIFY(node == nullptr);
        node = new T;
    } else  if (testData.callCount == 1) {
        // 2. The second time we'll delete it and return nullptr.
        // This should cause any link to this object to be removed from the node map (On next sync).
        QVERIFY(node != nullptr);
        node = nullptr;
    } else if (testData.callCount == 2) {
        // 3. The third time we verify that the received node is nullptr (It was indeed uppdate)
        //    and then we'll create a new node.
        QVERIFY(node == nullptr);
        // There should be no links to this object in the node map
        const auto values = nodeMap.values();
        QVERIFY(!values.contains(owner));
        node = new T;
        testData.finalNode = node;
    } else if (testData.callCount == 3) {
        // The forth time we verify that the received node is the same as the new one.
        QCOMPARE(node, testData.finalNode);
    } else {
        Q_UNREACHABLE();
    }

    testData.callCount++;

    // as long as the count is less then the expected could we trigger an update
    const bool done = testData.callCount >= testData.expectedCallCount;
    if (!done)
        QMetaObject::invokeMethod(owner, "update", Qt::QueuedConnection);
}


/// RESOURCE OBJECT
///
///

TestData testData_ResourceObject { 3 };

class MyCustomGraphObject : public QSSGRenderGraphObject
{
public:
    MyCustomGraphObject() : QSSGRenderGraphObject(Type(BaseType::Resource)) {}
    ~MyCustomGraphObject() override
    {
        testData_ResourceObject.destroyedCount++;
    }
};

MyCustomResource::MyCustomResource(QQuick3DObject *parent)
    : QQuick3DObject(parent)
{
}

QSSGRenderGraphObject *MyCustomResource::updateSpatialNode(QSSGRenderGraphObject *node)
{
    tst_updateSpatialNode<MyCustomGraphObject>(this, testData_ResourceObject, node);

    return node;
}

/// NODE OBJECT
///
///

TestData testData_NodeObject { 3 };

class MyCustomRenderNode : public QSSGRenderNode
{
public:
    MyCustomRenderNode() : QSSGRenderNode() {}
    ~MyCustomRenderNode() override
    {
        testData_NodeObject.destroyedCount++;
    }
};

MyCustomNode::MyCustomNode(QQuick3DNode *parent)
    : QQuick3DNode(parent)
{

}

QSSGRenderGraphObject *MyCustomNode::updateSpatialNode(QSSGRenderGraphObject *node)
{
    tst_updateSpatialNode<MyCustomRenderNode>(this, testData_NodeObject, node);

    return node;
}

/// EXTENSION OBJECT
///
///

TestData testData_ExtensionObject { 3 };

class MyCustomExtensionImpl : public QSSGRenderExtension
{
public:
    MyCustomExtensionImpl() {}
    ~MyCustomExtensionImpl() override
    {
        testData_ExtensionObject.destroyedCount++;
    }

    bool prepareData(QSSGFrameData &data) override { Q_UNUSED(data); return true; }
    void prepareRender(QSSGFrameData &data) override { Q_UNUSED(data); }
    void render(QSSGFrameData &data) override { Q_UNUSED(data); }
    void resetForFrame() override {}

    RenderMode mode() const override { return RenderMode::Standalone; }
    RenderStage stage() const override { return RenderStage::PreColor; }
};

MyCustomExtension::MyCustomExtension(QQuick3DObject *parent)
    : QQuick3DRenderExtension(parent)
{

}

QSSGRenderGraphObject *MyCustomExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    tst_updateSpatialNode<MyCustomExtensionImpl>(this, testData_ExtensionObject, node);

    return node;
}

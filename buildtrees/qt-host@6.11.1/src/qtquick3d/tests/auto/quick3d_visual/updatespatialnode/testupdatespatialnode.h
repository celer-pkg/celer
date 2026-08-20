// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_UPDATESPATIALNODE_H
#define TEST_UPDATESPATIALNODE_H

#include <QtQmlIntegration>

#include <QtQuick3D/qquick3dobject.h>

#include <QtQuick3D/private/qquick3dnode_p.h>

#include <QtQuick3D/qquick3drenderextensions.h>

QT_BEGIN_NAMESPACE

class QSSGRenderGraphObject;

QT_END_NAMESPACE

class TestData
{
public:
    explicit TestData(int expectedCallCount)
        : expectedCallCount(expectedCallCount)
    {
    }

    QSSGRenderGraphObject *finalNode = nullptr;

    const int expectedCallCount = -1;
    int callCount = 0;
    int destroyedCount = 0;
};


// RESOURCE OBJECT
//
//

class MyCustomResource : public QQuick3DObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit MyCustomResource(QQuick3DObject *parent = nullptr);
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
};

// NODE OBJECT
//
//

class MyCustomNode : public QQuick3DNode
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit MyCustomNode(QQuick3DNode *parent = nullptr);
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
};

// EXTENSION OBJECT
//
//

class MyCustomExtension : public QQuick3DRenderExtension
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit MyCustomExtension(QQuick3DObject *parent = nullptr);
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
};

#endif // TEST_UPDATESPATIALNODE_H

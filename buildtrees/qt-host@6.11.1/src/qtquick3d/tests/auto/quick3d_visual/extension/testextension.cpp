// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testextension.h"
#include <ssg/qssgrenderextensions.h>
#include <ssg/qssgrenderhelpers.h>
#include <QThread>
#include <QTest>

int extensionCtorCount = 0;
int extensionDtorCount = 0;
int extensionFunctional = 0;
int childExtensionCtorCount = 0;
int childExtensionDtorCount = 0;
int childExtensionFunctional = 0;

class ChildExtension : public QQuick3DRenderExtension
{
public:
    ChildExtension(QQuick3DObject *parent = nullptr)
        : QQuick3DRenderExtension(parent)
    {
    }

    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
};

class ChildRenderer : public QSSGRenderExtension
{
public:
    ChildRenderer();
    ~ChildRenderer();

    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;
    RenderMode mode() const override { return RenderMode::Main; }
    RenderStage stage() const override { return RenderStage::PostColor; }

    QThread *t;
};

QSSGRenderGraphObject *ChildExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        node = new ChildRenderer;

    return node;
}

ChildRenderer::ChildRenderer()
{
    t = QThread::currentThread();
    childExtensionCtorCount += 1;
}

ChildRenderer::~ChildRenderer()
{
    // There is no ordering guarantee when it comes to the destruction of nodes.
    // The Renderer (the other extension backend node) may or may not be alive at this point.

    // with the threaded render loop it is important that the dtor is called on the same thread as the ctor
    QCOMPARE(QThread::currentThread(), t);

    childExtensionDtorCount += 1;
}

bool ChildRenderer::prepareData(QSSGFrameData &data)
{
    auto camera = data.activeCamera();
    if (camera == QSSGCameraId::Invalid)
        qWarning("No camera in prepareData");

    QMatrix4x4 vp = QSSGCameraHelpers::getViewProjectionMatrix(camera);
    if (!vp.isIdentity())
        childExtensionFunctional += 1;
    else
        qWarning("Camera's view-projection matrix does not look valid.");

    return true;
}

void ChildRenderer::prepareRender(QSSGFrameData &)
{
    QVERIFY(hasGraphicsResources()); // should be set implicitly

    childExtensionFunctional += 1;
}

void ChildRenderer::render(QSSGFrameData &)
{
    childExtensionFunctional += 1;
}

void ChildRenderer::resetForFrame()
{
}

class Renderer : public QSSGRenderExtension
{
public:
    Renderer();
    ~Renderer();

    bool prepareData(QSSGFrameData &data) override;
    void prepareRender(QSSGFrameData &data) override;
    void render(QSSGFrameData &data) override;
    void resetForFrame() override;
    RenderMode mode() const override { return RenderMode::Standalone; }
    RenderStage stage() const override { return RenderStage::PostColor; }

    QThread *t;
};

MyExtension::MyExtension(QQuick3DObject *parent)
    : QQuick3DRenderExtension(parent)
{
    new ChildExtension(this);
}

QSSGRenderGraphObject *MyExtension::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (!node)
        node = new Renderer;

    return node;
}

void MyExtension::markDirty(Dirty v)
{
    m_dirtyFlag |= v;
    update();
}

Renderer::Renderer()
{
    t = QThread::currentThread();
    extensionCtorCount += 1;
}

Renderer::~Renderer()
{
    // There is no ordering guarantee when it comes to the destruction of nodes.
    // The ChildRenderer (the other extension backend node) may or may not be alive at this point.

    // with the threaded render loop it is important that the dtor is called on the same thread as the ctor
    // NOTE: We only want to check that the address is the same (the object might have been destroyed already)...
    QVERIFY(QThread::currentThread() == t);

    extensionDtorCount += 1;
}

bool Renderer::prepareData(QSSGFrameData &data)
{
    auto camera = data.activeCamera();
    if (camera == QSSGCameraId::Invalid)
        qWarning("No camera in prepareData");
    else if (childExtensionFunctional == 0) // the order is undefined for siblings, and bottom-up for parent-child relationships
        qWarning("Child did not run before parent's prepareData");
    else
        extensionFunctional += 1;

    return true;
}

void Renderer::prepareRender(QSSGFrameData &)
{
    QVERIFY(hasGraphicsResources());

    if (childExtensionFunctional == 0)
        qWarning("Child did not run before parent's prepareRender");
    else
        extensionFunctional += 1;
}

void Renderer::render(QSSGFrameData &)
{
    if (childExtensionFunctional == 0)
        qWarning("Child did not run before parent's render");
    else
        extensionFunctional += 1;
}

void Renderer::resetForFrame()
{
}

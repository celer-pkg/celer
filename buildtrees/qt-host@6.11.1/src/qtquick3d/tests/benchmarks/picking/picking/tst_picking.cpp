// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>

#include <ssg/qssgrendercontextcore.h>

#include <QtQuick3DRuntimeRender/private/qssgrhicustommaterialsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgdebugdrawsystem_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendershadercodegenerator_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderer_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercamera_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderpickresult_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendermodel_p.h>

class picking : public QObject
{
    Q_OBJECT

public:
    picking();
    ~picking() = default;

private Q_SLOTS:
    void initTestCase();
    void bench_picking1();
    void bench_picking1Miss();
    void bench_picking1in1k();
    void bench_picking1in1kMiss();

private:
    std::unique_ptr<QSSGRenderContextInterface> renderCtx;

    void benchImpl(int count, bool hit);
};

picking::picking()
    : renderCtx(std::make_unique<QSSGRenderContextInterface>(std::make_unique<QSSGBufferManager>()
                                                             , std::make_unique<QSSGRenderer>()
                                                             , nullptr
                                                             , nullptr
                                                             , nullptr
                                                             , nullptr
                                                             , std::make_unique<QSSGRhiContext>(QRhi::create(QRhi::Implementation::Null, nullptr))))
{
}

void picking::initTestCase()
{
    QSKIP("Test does not work with the RHI implementation at the moment");
}

void picking::bench_picking1()
{
    benchImpl(1, true);
}

void picking::bench_picking1Miss()
{
    benchImpl(1, false);
}

void picking::bench_picking1in1k()
{
    benchImpl(1000, true);
}

void picking::bench_picking1in1kMiss()
{
    benchImpl(1, false);
}

void picking::benchImpl(int count, bool hit)
{
    Q_ASSERT(count > 0 && count <= 1000);
    const auto &bufferManager = renderCtx->bufferManager();
    QVector2D viewportDim(400.0f, 400.0f);
    QSSGRenderLayer dummyLayer;
    QMatrix4x4 globalTransform;
    QMatrix4x4 viewProjection;

    QSSGRenderCamera dummyCamera(QSSGRenderCamera::Type::OrthographicCamera);
    dummyCamera.localTransform.translate(QVector3D(0.0f, 0.0f, 600.0f));
    static_cast<QSSGRenderNode &>(dummyCamera).markDirty(QSSGRenderNode::DirtyFlag::TransformDirty);
    QSSGRenderCamera::calculateProjectionInternal(dummyCamera, QRectF(QPointF(), QSizeF(viewportDim.x(), viewportDim.y())));
    dummyCamera.calculateViewProjectionMatrix(globalTransform, viewProjection);

    dummyLayer.renderedCameras = { &dummyCamera };

    static const auto setModelPosition = [](QSSGRenderModel &model, const QVector3D &pos) {
        model.localTransform.translate(pos);
        model.markDirty(QSSGRenderNode::DirtyFlag::TransformDirty);
    };

    QSSGRenderModel models[1000];

    const auto cubeMeshPath = QSSGRenderPath(QStringLiteral("#Cube"));

    for (int i = 0; i != count; ++i) {
        auto &model = models[i];
        model.meshPath = cubeMeshPath;
        model.setState(QSSGRenderModel::LocalState::Pickable);
        setModelPosition(model, { 0.0f + i, 0.0f + i, 0.0f + i });
        dummyLayer.addChild(model);
    }

    // Since we're using the same mesh for each model, we only need to call loadMesh() once.
    bufferManager->loadMesh(models[0]);

    QVarLengthArray<QSSGRenderPickResult, 20> res;
    QSSGRenderRay ray = hit ? QSSGRenderRay{ { 0.0f, 0.0f, -100.0f }, { 0.0f, 0.0f, 1.0f } } : QSSGRenderRay{ { 0.0f, 0.0f, -100.0f }, { 1.0f, 0.0f, 0.0f } };
    QBENCHMARK {
        res = QSSGRendererPrivate::syncPick(*renderCtx, dummyLayer, ray);
    }
    QVERIFY(!res.isEmpty());
    QVERIFY(res.first().m_hitObject != nullptr);
}

QTEST_APPLESS_MAIN(picking)

#include "tst_picking.moc"

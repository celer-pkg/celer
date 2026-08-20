// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssglightmapbaker_p.h"

#ifdef QT_QUICK3D_HAS_LIGHTMAPPER
#include <QString>
#include <QtQuick3DRuntimeRender/private/qssglayerrenderdata_p.h>
#include <QThreadPool>
#include "qssgrendercontextcore.h"
#if QT_CONFIG(opengl)
#include <QOffscreenSurface>
#endif // QT_CONFIG(opengl)

QT_BEGIN_NAMESPACE

struct QSSGLightmapBakerPrivate
{
    QSSGLightmapBaker::Context ctx;
    std::unique_ptr<QSSGLightmapper> lightmapper = nullptr;
    QSSGLightmapBaker::Status currentStatus = QSSGLightmapBaker::Status::Preparing;
    // Use a local threadpool to be able to set highest thread priority on the bake thread
    QThreadPool localThreadPool;
    QOffscreenSurface *fallbackSurface = nullptr;
    bool waitingForInvoke = false;
};

QSSGLightmapBaker::QSSGLightmapBaker(const QSSGLightmapBaker::Context &ctx)
    : d(new QSSGLightmapBakerPrivate)
{
    d->ctx = ctx;
    d->currentStatus = QSSGLightmapBaker::Status::Preparing;
    d->localThreadPool.setMaxThreadCount(1);
}

QSSGLightmapBaker::~QSSGLightmapBaker()
{
    delete d;
}

QSSGLightmapBaker::Status QSSGLightmapBaker::process()
{
    if (d->waitingForInvoke)
        return d->currentStatus;

    auto &env = d->ctx.env;
    auto &settings = d->ctx.settings;
    auto &callbacks = d->ctx.callbacks;

    if (d->currentStatus == Status::Preparing) {
        // We need to prepare for lightmap baking by doing another frame so that
        // we can reload all meshes to use the original one and NOT the baked one.
        // When disableLightmaps is set on the layer, the mesh loader will always load the
        // original mesh and not the lightmap mesh.
        callbacks.setCurrentlyBaking(true);
        callbacks.triggerNewFrame(true);

#if QT_CONFIG(opengl)
        d->waitingForInvoke = true;
        QMetaObject::invokeMethod(qApp, [this]() {
            d->fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
            d->currentStatus = Status::Running;
            d->waitingForInvoke = false;
        },
        Qt::QueuedConnection);
#else
        d->currentStatus = Status::Running;
#endif
    } else if (d->currentStatus == Status::Running) {
        d->lightmapper = std::make_unique<QSSGLightmapper>();
        d->lightmapper->setRhiBackend(env.rhiCtx->rhi()->backend());
        d->lightmapper->setOptions(env.lmOptions);
        d->lightmapper->setOutputCallback(callbacks.lightmapBakingOutput);
        d->lightmapper->setDenoiseOnly(settings.denoiseRequested);

        // bakedLightingModels contains all models with
        // usedInBakedLighting: true. These, together with lights that
        // have a bakeMode set to either Indirect or All, form the
        // lightmapped scene. A lightmap is stored persistently only
        // for models that have their lightmapKey set.
        const auto &bakedLightingModels = callbacks.modelsToBake();
        for (int i = 0, ie = bakedLightingModels.size(); i != ie; ++i)
            d->lightmapper->add(bakedLightingModels[i]);

        if (!d->lightmapper->setupLights(*env.renderer)) {
            d->currentStatus = Status::Finished;
            callbacks.setCurrentlyBaking(false);
            callbacks.triggerNewFrame(true);
            if (settings.quitWhenFinished) {
                qDebug("Lightmap baking/denoising done, exiting application");
                QMetaObject::invokeMethod(qApp, "quit");
            }
            return Status::Finished;
        }

        QRhiCommandBuffer *cb = env.rhiCtx->commandBuffer();
        cb->debugMarkBegin("Quick3D lightmap baking/denoising");

        d->currentStatus = Status::Baking;

        const auto bakeTask = [this, callbacks, settings] {
#if QT_CONFIG(thread)
            QThread::currentThread()->setPriority(QThread::HighestPriority);
#endif // QT_CONFIG(thread)
            d->lightmapper->run(d->fallbackSurface);

            callbacks.setCurrentlyBaking(false);
            callbacks.triggerNewFrame(true);

#if QT_CONFIG(opengl)
            d->waitingForInvoke = true;
            QMetaObject::invokeMethod(qApp, [this]() {
                delete d->fallbackSurface;
                d->fallbackSurface = nullptr;
                d->currentStatus = Status::Finished;
                d->waitingForInvoke = false;
            }, Qt::QueuedConnection);
#else
            d->currentStatus = Status::Finished;
#endif
            if (settings.quitWhenFinished) {
                qDebug("Lightmap baking/denoising done, exiting application");
                QMetaObject::invokeMethod(qApp, "quit");
            }
        };

#if QT_CONFIG(thread)
        d->localThreadPool.start(bakeTask);
#else
        bakeTask();
#endif // QT_CONFIG(thread)

        // Wait until lightmapper is finished initializing
        d->lightmapper->waitForInit();
        cb->debugMarkEnd();
    }

    return d->currentStatus;
}

QT_END_NAMESPACE

#else // QT_QUICK3D_HAS_LIGHTMAPPER

QT_BEGIN_NAMESPACE

QSSGLightmapBaker::QSSGLightmapBaker(const QSSGLightmapBaker::Context &ctx)
{
    qWarning("QSSGLightmapBaker: This build has no lightmap baking support.");
#if defined(qApp)
    if (ctx.settings.quitWhenFinished)
        QMetaObject::invokeMethod(qApp, "quit");
#else
    Q_UNUSED(ctx);
#endif
}

QSSGLightmapBaker::~QSSGLightmapBaker() = default;

QSSGLightmapBaker::Status QSSGLightmapBaker::process()
{
    return QSSGLightmapBaker::Status::Finished;
}

QT_END_NAMESPACE

#endif // QT_QUICK3D_HAS_LIGHTMAPPER

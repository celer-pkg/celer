// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGLIGHTMAPBAKER_P_H
#define QSSGLIGHTMAPBAKER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DRuntimeRender/private/qtquick3druntimerenderglobal_p.h>
#include <QtQuick3DRuntimeRender/private/qssglightmapper_p.h>

#include <QString>

QT_BEGIN_NAMESPACE

class QSSGLayerRenderData;
struct QSSGLightmapBakerPrivate;

class QSSGLightmapBaker
{
public:
    enum class Status {
        Preparing,
        Running,
        Baking,
        Finished
    };

    struct Context {
        struct Environment {
            QSSGRhiContext *rhiCtx = nullptr;
            QSSGRenderer *renderer = nullptr;
            QSSGLightmapperOptions lmOptions;
        } env;

        struct Settings {
            bool bakeRequested = false;
            bool denoiseRequested = false;
            bool quitWhenFinished = false;
        } settings;

        struct Callbacks {
            QSSGLightmapper::Callback lightmapBakingOutput;
            std::function<QVector<QSSGBakedLightingModel>()> modelsToBake;
            std::function<void(bool)> triggerNewFrame;
            std::function<void(bool)> setCurrentlyBaking;
        } callbacks;
    };

    QSSGLightmapBaker(const Context &ctx);
    ~QSSGLightmapBaker();

    Status process();

private:
#ifdef QT_QUICK3D_HAS_LIGHTMAPPER
    QSSGLightmapBakerPrivate *d;
#endif
};

QT_END_NAMESPACE

#endif

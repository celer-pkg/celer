// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGLIGHTMAPPER_P_H
#define QSSGLIGHTMAPPER_P_H

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
#include <rhi/qrhi.h>
#include <ssg/qssglightmapper.h>

#include <QString>

QT_BEGIN_NAMESPACE

struct QSSGLightmapperPrivate;
struct QSSGBakedLightingModel;
class QSSGRhiContext;
class QSSGRenderer;
struct QSSGRenderModel;
class QOffscreenSurface;

class QSSGLightmapper
{
public:
    enum class BakingStatus {
        None,
        Info,
        Warning,
        Error,
        Cancelled,
        Failed,
        Complete
    };

    struct BakingControl {
        bool cancelled = false;
    };

    /*
     * Payload:
     *  int    status               (BakingStatus)
     *  string stage
     *  string message
     *  qint64 totalTimeRemaining   (in ms)
     *  double totalProgress        (Range 0 - 1)
     */
    typedef std::function<void(const QVariantMap &payload, BakingControl*)> Callback;

    QSSGLightmapper();
    ~QSSGLightmapper();
    void reset();

    bool setupLights(const QSSGRenderer &renderer);
    void setOptions(const QSSGLightmapperOptions &options);
    void setOutputCallback(Callback callback);
    qsizetype add(const QSSGBakedLightingModel &model);
    void setRhiBackend(QRhi::Implementation backend);
    void setDenoiseOnly(bool value);

    // NOTE: since add() contains references to objects in the
    // running scene we need to call the functions in the following order:
    // add(), run(), waitForInit().
    // OpenGL requires a fallback surface created on the main thread to create the
    // RHI object, so it is provided as a pointer.
    void run(QOffscreenSurface *fallbackSurface);
    // waitForInit() waits until all models have been processed and are therefore
    // not referenced anymore and it is safe to go back to rendering the scene.
    // This should be called after run(), otherwise it will be stuck in a deadlock.
    void waitForInit();

private:
#ifdef QT_QUICK3D_HAS_LIGHTMAPPER
    QSSGLightmapperPrivate *d = nullptr;
#endif
    bool bake();
    bool denoise();
};

QT_END_NAMESPACE

#endif

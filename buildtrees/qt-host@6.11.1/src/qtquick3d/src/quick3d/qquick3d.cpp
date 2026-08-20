// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3d.h"

#if QT_CONFIG(opengl)
# include <QtGui/qopenglcontext.h>
#endif
#include <QtQuick/qquickwindow.h>

#include <QtCore/qloggingcategory.h>

Q_STATIC_LOGGING_CATEGORY(lcQuick3D, "qt.quick3d.general")

/*!
    \class QQuick3D
    \inmodule QtQuick3D
    \since 5.15
    \brief Helper class for selecting correct surface format.

    When using Qt Quick 3D with OpenGL it is necessary to take extra steps to
    define what kind of \l {QSurfaceFormat}{surface format} is used when
    initializing Qt Quick. This is because by the time Qt Quick is aware that
    3D content is being used, the OpenGL context and window surface has already
    been initialized. So this helper class is provided to request the ideal
    surface format from Qt Quick 3D so that it can be set as the default surface
    for Qt Quick before initialization.

    If this helper is run when using any other rendering backends than OpenGL
    then it just returns a copy of the current default QSurfaceFormat with the
    requested samples.

    If this helper is run when using the OpenGL rendering backend, then it will
    test for sufficiently modern versions of OpenGL and support for
    multisampling if requested. Normally Qt Quick will request an OpenGL 2.0 or
    OpenGL ES 2.0 context, which would limit the features available when using
    Qt Quick 3D, so an extra step is needed to request a more capable context.

    The correct usage pattern is to call \l QSurfaceFormat::setDefaultFormat
    to set the \l QSurfaceFormat returned by \l QQuick3D::idealSurfaceFormat.
    It is important that this method is called after \l QGuiApplication is
    constructed, but before the Qt Quick application content is loaded. This
    code snippet shows the correct usage pattern:
    \code
    #include <QGuiApplication>
    #include <QQmlApplicationEngine>

    #include <QtGui>
    #include <QtQuick3D/qquick3d.h>

    int main(int argc, char *argv[])
    {
        QGuiApplication app(argc, argv);

        QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat(4));

        QQmlApplicationEngine engine;
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        if (engine.rootObjects().isEmpty())
            return -1;

        return app.exec();
    }
    \endcode

    \sa {Qt Quick 3D Graphics Requirements} {OpenGL specifics}
*/

QT_BEGIN_NAMESPACE
#if QT_CONFIG(opengl)
static QSurfaceFormat findIdealGLVersion(int samples)
{
    QSurfaceFormat fmt;
    int defaultSamples = fmt.samples();
    const bool multisampling = samples > 1;
    fmt.setProfile(QSurfaceFormat::CoreProfile);

    // Proper case: Try 4.3 core (so we get compute shaders for instance)
    fmt.setVersion(4, 3);
    fmt.setSamples(multisampling ? samples : defaultSamples);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);
    if (ctx.create() && ctx.format().version() >= qMakePair(4, 3)) {
        qCDebug(lcQuick3D, "Requesting OpenGL 4.3 core context succeeded");
        return ctx.format();
    }
    if (multisampling) {
        // try without multisampling
        fmt.setSamples(defaultSamples);
        ctx.setFormat(fmt);
        if (ctx.create() && ctx.format().version() >= qMakePair(4, 3)) {
            qCDebug(lcQuick3D, "Requesting OpenGL 4.3 core context succeeded without multisampling");
            return ctx.format();
        }
    }

    // Fallback, but still good, case: Stick with 3.3 (the only thing we lose is compute for HDR mipmaps)
    fmt.setVersion(3, 3);
    fmt.setSamples(multisampling ? samples : defaultSamples);
    ctx.setFormat(fmt);
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 3)) {
        qCDebug(lcQuick3D, "Requesting OpenGL 3.3 core context succeeded");
        return ctx.format();
    }
    if (multisampling) {
        // try without multisampling
        fmt.setSamples(defaultSamples);
        ctx.setFormat(fmt);
        if (ctx.create() && ctx.format().version() >= qMakePair(3, 3)) {
            qCDebug(lcQuick3D, "Requesting OpenGL 3.3 core context succeeded without multisampling");
            return ctx.format();
        }
    }

    // If all else fails, try 3.0. This may have some issues but most things should work.
    fmt.setVersion(3, 0);
    // the modern core-compat. concept as we know it is only there since 3.2
    fmt.setProfile(QSurfaceFormat::NoProfile);
    fmt.setSamples(multisampling ? samples : defaultSamples);
    ctx.setFormat(fmt);
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 0)) {
        qCDebug(lcQuick3D, "Requesting OpenGL 3.0 context succeeded");
        return ctx.format();
    }
    if (multisampling) {
        fmt.setSamples(defaultSamples);
        ctx.setFormat(fmt);
        if (ctx.create() && ctx.format().version() >= qMakePair(3, 0)) {
            qCDebug(lcQuick3D, "Requesting OpenGL 3.0 context succeeded without multisampling");
            return ctx.format();
        }
    }

    qCWarning(lcQuick3D, "Unable to find ideal GL version.");
    return fmt;
}

static QSurfaceFormat findIdealGLESVersion(int samples)
{
    QSurfaceFormat fmt;
    int defaultSamples = fmt.samples();
    const bool multisampling = samples > 1;

    // Proper case: Try 3.1 (so we get compute shaders)
    fmt.setVersion(3, 1);
    fmt.setRenderableType(QSurfaceFormat::OpenGLES);
    fmt.setSamples(multisampling ? samples : defaultSamples);
    QOpenGLContext ctx;
    ctx.setFormat(fmt);

    // Now, it's important to check the format with the actual version (parsed
    // back from GL_VERSION) since some implementations are broken and succeed
    // the 3.1 context request even though they only support and return a 3.0
    // context. This is against the spec since 3.0 is obviously not backwards
    // compatible with 3.1, but hey...
    qCDebug(lcQuick3D, "Testing OpenGL ES 3.1");
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 1)) {
        qCDebug(lcQuick3D, "Requesting OpenGL ES 3.1 context succeeded");
        return ctx.format();
    }
    if (multisampling) {
        fmt.setSamples(defaultSamples);
        ctx.setFormat(fmt);
        if (ctx.create() && ctx.format().version() >= qMakePair(3, 1)) {
            qCDebug(lcQuick3D, "Requesting OpenGL ES 3.1 context succeeded without multisampling");
            return ctx.format();
        }
    }

    // Fallback, but still good, case: OpenGL ES 3.0
    fmt.setVersion(3, 0);
    fmt.setSamples(multisampling ? samples : defaultSamples);
    ctx.setFormat(fmt);
    qCDebug(lcQuick3D, "Testing OpenGL ES 3.0");
    if (ctx.create() && ctx.format().version() >= qMakePair(3, 0)) {
        qCDebug(lcQuick3D, "Requesting OpenGL ES 3.0 context succeeded");
        return ctx.format();
    }
    if (multisampling) {
        fmt.setSamples(defaultSamples);
        ctx.setFormat(fmt);
        if (ctx.create() && ctx.format().version() >= qMakePair(3, 0)) {
            qCDebug(lcQuick3D, "Requesting OpenGL ES 3.0 context succeeded without multisampling");
            return ctx.format();
        }
    }

    // If all else fails, try 2.0 but that's going to lose a bunch of features.
#ifdef Q_OS_WASM
    qCWarning(lcQuick3D, "OpenGL ES 3.0 / WebGL 2 is required on WebAssembly.");
#endif
    fmt.setVersion(2, 0);
    fmt.setSamples(multisampling ? samples : defaultSamples);
    ctx.setFormat(fmt);
    qCDebug(lcQuick3D, "Testing OpenGL ES 2.0");
    if (ctx.create()) {
        qCDebug(lcQuick3D, "Requesting OpenGL ES 2.0 context succeeded");
        return fmt;
    }
    if (multisampling) {
        fmt.setSamples(defaultSamples);
        ctx.setFormat(fmt);
        if (ctx.create()) {
            qCDebug(lcQuick3D, "Requesting OpenGL ES 2.0 context succeeded without multisampling");
            return fmt;
        }
    }

    qCWarning(lcQuick3D, "Unable to find ideal GLES version.");
    return fmt;
}
#endif // #if QT_CONFIG(opengl)
/*!
    Returns an ideal surface format for the platform. Optionally, \a samples can be specified
    to select the number of multisamples for antialiasing.
*/
QSurfaceFormat QQuick3D::idealSurfaceFormat(int samples)
{
    if (QQuickWindow::graphicsApi() != QSGRendererInterface::OpenGLRhi) {
        QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
        fmt.setSamples(samples);
        return fmt;
    }
#if QT_CONFIG(opengl)
    static const QSurfaceFormat f = [samples] {
        QSurfaceFormat fmt;
        if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) { // works in dynamic gl builds too because there's a qguiapp already
            fmt = findIdealGLVersion(samples);
        } else {
            fmt = findIdealGLESVersion(samples);
        }
        fmt.setDepthBufferSize(24);
        fmt.setStencilBufferSize(8);
        // Ignore MSAA here as that is a per-layer setting.
        return fmt;
    }();
#else
    // It really shouldn't be possible to get but if we do
    // but at least return something if we do.
    QSurfaceFormat f = QSurfaceFormat::defaultFormat();
#endif //#if QT_CONFIG(opengl)
    return f;
}

QT_END_NAMESPACE

// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuick3DObject>
#include <QVector3D>
#include <QCommandLineParser>
#include <QQuickWindow>
#include <private/qquick3dxrview_p.h>
#include <private/qquick3dsceneenvironment_p.h>
#include <rhi/qrhi.h>

#include <QtCore/qloggingcategory.h>

#ifdef Q_OS_WIN
#  include <fcntl.h>
#  include <io.h>
#endif // Q_OS_WIN

// A valid screen grab requires the scene to not change
// for SCENE_STABLE_TIME ms
#define SCENE_STABLE_TIME 200

static int getLancelotSceneTimeout()
{
    static int sceneTimeout = qEnvironmentVariableIntValue("LANCELOT_SCENE_TIMEOUT");
    if (!sceneTimeout)
        sceneTimeout = 6000; // Default value in ms
    return sceneTimeout;
}

static bool writeOutGrabbedFrame(QImage lastGrab, QSize outputSize, const QString &ofile)
{
    if (outputSize.isValid())
        lastGrab = lastGrab.scaled(outputSize, Qt::KeepAspectRatio);

    if (ofile == "-") {   // Write to stdout
        QFile of;
#ifdef Q_OS_WIN
        // Make sure write to stdout doesn't do LF->CRLF
        _setmode(_fileno(stdout), _O_BINARY);
#endif // Q_OS_WIN
        if (!of.open(1, QIODevice::WriteOnly) || !lastGrab.save(&of, "ppm"))
            qWarning() << "Error: failed to write grabbed image to stdout.";
    } else {
        if (!lastGrab.save(ofile))
            qWarning() << "Error: failed to store grabbed image to" << ofile;
    }

    return true;
}

enum class MultiviewOutputStrategy {
    Single,
    Left,
    Right,
    Separate,
    Merged
};

template <MultiviewOutputStrategy Strategy>
static QString generateOfName(int frame)
{
    if constexpr (Strategy == MultiviewOutputStrategy::Left)
        return QString::asprintf("frame_left_%d.png", frame);
    if constexpr (Strategy == MultiviewOutputStrategy::Right)
        return QString::asprintf("frame_right_%d.png", frame);
    if constexpr (Strategy == MultiviewOutputStrategy::Merged)
        return QString::asprintf("frame_multiview_%d.png", frame);

    return QString::asprintf("frame_%d.png", frame);
}

template <MultiviewOutputStrategy Strategy>
static bool writeOutMultiview(const QImage &left, const QImage &right, QSize outputSize, int frame, QString ofile)
{
    // Assumes the images are of the same size
    if (!outputSize.isValid())
        outputSize = left.size();
    if (!outputSize.isValid())
        outputSize = right.size();

    if constexpr (Strategy != MultiviewOutputStrategy::Separate) {
        if (ofile.isEmpty())
            ofile = generateOfName<Strategy>(frame);
    }

    if constexpr (Strategy == MultiviewOutputStrategy::Left) {
        Q_UNUSED(right);
        return writeOutGrabbedFrame(left, outputSize, ofile);
    }

    if constexpr (Strategy == MultiviewOutputStrategy::Right) {
        Q_UNUSED(left);
        return writeOutGrabbedFrame(right, outputSize, ofile);
    }

    if constexpr (Strategy == MultiviewOutputStrategy::Separate) {
        if (ofile.size() == 1) {
            qWarning() << "Error: separate output strategy requires two output files";
            return false;
        }

        QString ofileLeft;
        QString ofileRight;

        if (ofile.isEmpty()) {
            ofileLeft = generateOfName<MultiviewOutputStrategy::Left>(frame);
            ofileRight = generateOfName<MultiviewOutputStrategy::Right>(frame);
        } else {
            ofileLeft = QStringLiteral("left_") + ofile;
            ofileRight = QStringLiteral("right_") + ofile;
        }

        if (!writeOutGrabbedFrame(left, outputSize, ofileLeft))
            return false;
        return writeOutGrabbedFrame(right, outputSize, ofileRight);
    }

    if constexpr (Strategy == MultiviewOutputStrategy::Merged) {
        QImage mergedImage(left.width() + right.width(), qMax(left.height(), right.height()), QImage::Format_ARGB32);
        QPainter painter(&mergedImage);
        painter.drawImage(0, 0, left);
        painter.drawImage(left.width(), 0, right);
        painter.end();
        return writeOutGrabbedFrame(mergedImage, outputSize, ofile);
    }
}

int main(int argc, char *argv[])
{
    QHashSeed::setDeterministicGlobalSeed();

    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache);
    QGuiApplication app(argc, argv);

    QUnifiedTimer::instance()->setConsistentTiming(true);

    QElapsedTimer elapsedTimer;

    QCommandLineParser cmdLineParser;
    // NOTE: Keep the same behavior as the non-XR scenegrabber which uses single dash word options.
    cmdLineParser.setSingleDashWordOptionMode(QCommandLineParser::SingleDashWordOptionMode::ParseAsLongOptions);
    const QString appDesc = QString::asprintf("qmlxr %s", qVersion());
    cmdLineParser.setApplicationDescription(appDesc);
    app.setApplicationVersion(QStringLiteral(QT_VERSION_STR));
    cmdLineParser.addHelpOption();
    cmdLineParser.addPositionalArgument(QStringLiteral("file"),
                                        QStringLiteral(".qml file with a Node root"),
                                        QStringLiteral("file"));

    QCommandLineOption disableMultiviewOption("disable-multiview", QStringLiteral("Disable multiview rendering (Enabled by default if supported)"));
    cmdLineParser.addOption(disableMultiviewOption);

    QCommandLineOption multiviewStrategyOption("output-strategy", QStringLiteral("Output <strategy> for captured multiview frames are outputed.\n <strategy> should be one of: left, right, separate, or merged (default)"),
                                               QStringLiteral("strategy"),
                                               QStringLiteral("merged"));
    cmdLineParser.addOption(multiviewStrategyOption);

    QCommandLineOption outputSizeOption({ "s", "size" }, QStringLiteral("Output size for the grabbed frame"), QStringLiteral("widthxheight"));
    cmdLineParser.addOption(outputSizeOption);

    QCommandLineOption shaderDebugOption("print-shaders", QStringLiteral("Print generated material shaders"));
    cmdLineParser.addOption(shaderDebugOption);

    QCommandLineOption captureOption({ "c", "capture" }, QStringLiteral("Take RenderDoc frame captures (qtbase must be built with RenderDoc support)"));
    cmdLineParser.addOption(captureOption);


    QCommandLineOption viewonlyOption({ "v", "viewonly"}, QStringLiteral("Only view the scene, do not grab frames"));
    cmdLineParser.addOption(viewonlyOption);

    QCommandLineOption outputOption({ "o", "output" }, QStringLiteral("Output file for the grabbed frame"), QStringLiteral("file"));
    cmdLineParser.addOption(outputOption);

    QCommandLineOption exitOption({ "x", "exit-after" }, QStringLiteral("Exit after <num_frames> frames"), QStringLiteral("num_frames"), QStringLiteral("1"));
    cmdLineParser.addOption(exitOption);

    QCommandLineOption debugOption({ "l", "validate" }, QStringLiteral("Enable D3D12/Vulkan/OpenXR debug or validation layer, if available. Also enables QRhi leak checking."));
    cmdLineParser.addOption(debugOption);

    QCommandLineOption msaaOption({ "a", "msaa" }, QStringLiteral("Request MSAA with <num_samples> samples. <num_samples> is 2 or 4."), QStringLiteral("num_samples"));
    cmdLineParser.addOption(msaaOption);

    QCommandLineOption showControllersOptions("show-controllers", QStringLiteral("Show controllers in the scene"));
    cmdLineParser.addOption(showControllersOptions);

    QCommandLineOption showRuntimeDialogOption("show-runtime-dialog", QStringLiteral("Show runtime dialog"));
    cmdLineParser.addOption(showRuntimeDialogOption);

    QCommandLineOption submitDepthOption({ "e", "submit-depth" }, QStringLiteral("Forces submitting the depth buffer (XR_KHR_composition_layer_depth), if supported."));
    cmdLineParser.addOption(submitDepthOption);

    QCommandLineOption nullOption("backend-null", QStringLiteral("Use QRhi backend Null"));
    cmdLineParser.addOption(nullOption);
    QCommandLineOption glOption("backend-opengl", QStringLiteral("Use QRhi backend OpenGL (ES)"));
    cmdLineParser.addOption(glOption);
    QCommandLineOption vkOption("backend-vulkan", QStringLiteral("Use QRhi backend Vulkan [default on Android]"));
    cmdLineParser.addOption(vkOption);
    QCommandLineOption d3d11Option("backend-d3d11", QStringLiteral("Use QRhi backend Direct3D 11"));
    cmdLineParser.addOption(d3d11Option);
    QCommandLineOption d3d12Option("backend-d3d12", QStringLiteral("Use QRhi backend Direct3D 12 [default on Windows]"));
    cmdLineParser.addOption(d3d12Option);
    QCommandLineOption mtlOption("backend-metal", QStringLiteral("Use QRhi backend Metal"));
    cmdLineParser.addOption(mtlOption);

    QCommandLineOption devOpt("q", QStringLiteral("Enable developer settings and logging"));
    cmdLineParser.addOption(devOpt);

    cmdLineParser.process(app);

    MultiviewOutputStrategy multiviewStrategy = MultiviewOutputStrategy::Merged;

    if (cmdLineParser.isSet(multiviewStrategyOption)) {
        const QString strategy = cmdLineParser.value(multiviewStrategyOption);
        if (strategy == QStringLiteral("left"))
            multiviewStrategy = MultiviewOutputStrategy::Left;
        else if (strategy == QStringLiteral("right"))
            multiviewStrategy = MultiviewOutputStrategy::Right;
        else if (strategy == QStringLiteral("separate"))
            multiviewStrategy = MultiviewOutputStrategy::Separate;
        else if (strategy == QStringLiteral("merged"))
            multiviewStrategy = MultiviewOutputStrategy::Merged;
        else {
            qWarning() << "Error: unknown output strategy" << strategy;
            return 1;
        }
    }

    QString ofile;

    if (cmdLineParser.isSet(outputOption)) {
        ofile = cmdLineParser.value(outputOption);
        if (ofile.isEmpty()) {
            qWarning() << "Error: output file is required in combination with the output option";
            return 1;
        }
    }

    QSize outputSize;
    if (cmdLineParser.isSet(outputSizeOption)) {
        const QStringList size = cmdLineParser.value(outputSizeOption).split('x');
        if (size.size() != 2) {
            qWarning() << "Error: output size must be in the format widthxheight";
            return 1;
        }
        outputSize = QSize(size[0].toInt(), size[1].toInt());
    }

    const auto positionalArguments = cmdLineParser.positionalArguments();
    if (positionalArguments.isEmpty()) {
        cmdLineParser.showHelp();
        return 0;
    }

    const QString filename = positionalArguments.first();

    const bool devOptSet = cmdLineParser.isSet(devOpt);

    if (devOptSet) {
        qputenv("QSG_INFO", "1");
        qputenv("QSG_RHI_PROFILE", "1");
        QLoggingCategory::setFilterRules(QStringLiteral("qt.quick3d.xr=true"));
    }

#ifdef Q_OS_WIN
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D12);
#endif
#ifdef Q_OS_ANDROID
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);
#endif

    if (cmdLineParser.isSet(nullOption))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Null);
    else if (cmdLineParser.isSet(glOption))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    else if (cmdLineParser.isSet(vkOption))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Vulkan);
    else if (cmdLineParser.isSet(d3d11Option))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
    else if (cmdLineParser.isSet(d3d12Option))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D12);
    else if (cmdLineParser.isSet(mtlOption))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Metal);

    qputenv("QT_QUICK_CONTROLS_STYLE", "Basic");

    if (cmdLineParser.isSet(disableMultiviewOption))
        qputenv("QT_QUICK3D_XR_DISABLE_MULTIVIEW", "1");

    if (cmdLineParser.isSet(shaderDebugOption))
        qputenv("QT_RHI_SHADER_DEBUG", "1");

    if (cmdLineParser.isSet(captureOption)) {
        qputenv("QT_QUICK3D_SHADER_DEBUG_INFO", "1");
        qputenv("QT_QUICK3D_XR_FRAME_CAPTURE", "1");
    }

    if (cmdLineParser.isSet(debugOption)) {
        qputenv("QSG_RHI_DEBUG_LAYER", "1");
        qputenv("QT_RHI_LEAK_CHECK", "1");
    }

    if (cmdLineParser.isSet(submitDepthOption))
        qputenv("QT_QUICK3D_XR_SUBMIT_DEPTH", "1");

    const bool viewOnly = cmdLineParser.isSet(viewonlyOption);

    QCoreApplication::setApplicationName("Qt XR Runner");
    QCoreApplication::setOrganizationName("The Qt Company");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/QtQuick3D/Lancelot/Xr/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url, viewOnly](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qWarning() << "Error: failed to load" << url;
            QCoreApplication::exit(-1);
        } else if (!viewOnly) {
            QTimer::singleShot(getLancelotSceneTimeout(), qApp, [] {
                qWarning() << "Error: timed out waiting for scene to stabilize!";
                QCoreApplication::exit(3);
            });
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    const QList<QObject *> rootObjects = engine.rootObjects();
    std::unique_ptr<QQmlComponent> component;
    int currentSingleViewEye = 0;
    int currentFrame = 1;
    int exitAfter = 1;
    if (cmdLineParser.isSet(exitOption))
        exitAfter = qMax(0, cmdLineParser.value(exitOption).toInt());

    if (!rootObjects.isEmpty()) {
        QQuick3DXrView *xrView = qobject_cast<QQuick3DXrView *>(rootObjects.first());

        if (cmdLineParser.isSet(showControllersOptions))
            xrView->setProperty("showControllers", true);

        if (cmdLineParser.isSet(showRuntimeDialogOption))
            xrView->setProperty("showRuntimeDialog", true);

        component.reset(new QQmlComponent(&engine, QUrl::fromLocalFile(filename)));
        if (component->isError()) {
            for (const QQmlError &error : component->errors()) {
                qWarning() << error.url() << error.line() << error;
                return 1;
            }
        }
        QObject *subRoot = component->create();
        if (component->isError()) {
            for (const QQmlError &error : component->errors()) {
                qWarning() << error.url() << error.line() << error;
                return 1;
            }
        }

        if (QQuick3DObject *obj = qobject_cast<QQuick3DObject *>(subRoot)) {
            obj->setParentItem(xrView);
            QObject *origin = xrView->findChild<QObject *>("xrorigin");
            QVariant posProp = obj->property("qmlxr_originPosition");
            if (!posProp.isNull()) {
                QVector3D camPos = posProp.value<QVector3D>();
                if (devOptSet)
                    qDebug() << "Setting origin position to" << camPos;
                origin->setProperty("position", camPos);
            }
            QVariant rotProp = obj->property("qmlxr_originRotation");
            if (!rotProp.isNull()) {
                QVector3D camRot = rotProp.value<QVector3D>();
                if (devOptSet)
                    qDebug() << "Setting origin rotation to" << camRot;
                origin->setProperty("eulerRotation", camRot);
            }
            QVariant envProp = obj->property("qmlxr_environment");
            if (!envProp.isNull()) {
                QQuick3DSceneEnvironment *env = envProp.value<QQuick3DSceneEnvironment *>();
                if (devOptSet)
                    qDebug() << "Setting SceneEnvironment to" << env;
                xrView->setEnvironment(env);
            }
        } else {
            qWarning() << subRoot << "is not a QQuick3DObject";
            return 1;
        }

        if (cmdLineParser.isSet(msaaOption)) {
            const int samples = qBound(1, cmdLineParser.value(msaaOption).toInt(), 8);
            if (samples > 1) {
                QQuick3DSceneEnvironment *sceneEnv = xrView->environment();
                if (devOptSet)
                    qDebug() << "Requesting MSAA with sample count" << samples << "on" << sceneEnv;
                sceneEnv->setAntialiasingMode(QQuick3DSceneEnvironment::MSAA);
                if (samples == 2)
                    sceneEnv->setAntialiasingQuality(QQuick3DSceneEnvironment::Medium);
                else if (samples == 4)
                    sceneEnv->setAntialiasingQuality(QQuick3DSceneEnvironment::High);
                else
                    sceneEnv->setAntialiasingQuality(QQuick3DSceneEnvironment::VeryHigh);
            }
        }

        const bool grab = !cmdLineParser.isSet(viewonlyOption) && !cmdLineParser.isSet(captureOption);

        if (grab) {
            QObject::connect(xrView, &QQuick3DXrView::frameReady, xrView, [&]() {
                if (!elapsedTimer.isValid())
                    elapsedTimer.start();
                if (elapsedTimer.elapsed() < SCENE_STABLE_TIME)
                    return;

                QRhiTexture *colorBuffer = nullptr;
                auto *view = QQuick3DXrViewPrivate::getView3d(xrView);
                if (view && view->window()) {
                    QQuickWindow *quickWindow = view->window();
                    QRhiRenderTarget *rt = QQuickWindowPrivate::get(quickWindow)->activeCustomRhiRenderTarget();
                    if (rt && rt->resourceType() == QRhiResource::TextureRenderTarget)
                        colorBuffer = static_cast<QRhiTextureRenderTarget *>(rt)->description().colorAttachmentAt(0)->texture();
                }
                if (!colorBuffer)
                    return;

                const int viewCount = qMax(1, colorBuffer->arraySize());

                if (!cmdLineParser.isSet(viewonlyOption)) {
                    QRhi *rhi = colorBuffer->rhi();
                    QRhiCommandBuffer *cb = nullptr;
                    if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess)
                        return;
                    QRhiResourceUpdateBatch *resourceUpdates = rhi->nextResourceUpdateBatch();
                    QVarLengthArray<QImage, 2> images(viewCount);
                    QVarLengthArray<QRhiReadbackResult, 2> results(viewCount);
                    QImage::Format imageFormat = QImage::Format_ARGB32;
                    for (int layer = 0; layer < viewCount; ++layer) {
                        results[layer].completed = [&results, &imageFormat, &images, rhi, layer] {
                            const uchar *p = reinterpret_cast<const uchar *>(results[layer].data.constData());
                            imageFormat = results[layer].format == QRhiTexture::BGRA8 ? QImage::Format_ARGB32 : QImage::Format_RGBA8888;
                            const QImage img(p, results[layer].pixelSize.width(), results[layer].pixelSize.height(), imageFormat);
                            // the order of the callbacks is undefined, hence must rely on the layer index
                            if (rhi->isYUpInFramebuffer())
                                images[layer] = img.flipped();
                            else
                                images[layer] = img.copy();
                        };
                        QRhiReadbackDescription readbackDesc(colorBuffer);
                        readbackDesc.setLayer(layer);
                        resourceUpdates->readBackTexture(readbackDesc, &results[layer]);
                    }
                    cb->resourceUpdate(resourceUpdates);
                    rhi->endOffscreenFrame();

                    if (images.count() >= 2) {
                        // multiview, this is the complete frame (2D texture array)
                        const auto &left = images.at(0);
                        const auto &right = images.at(1);

                        switch (multiviewStrategy) {
                        case MultiviewOutputStrategy::Single:
                            Q_FALLTHROUGH();
                        case MultiviewOutputStrategy::Left:
                            writeOutMultiview<MultiviewOutputStrategy::Left>(left, {}, outputSize, currentFrame, ofile);
                            break;
                        case MultiviewOutputStrategy::Right:
                            writeOutMultiview<MultiviewOutputStrategy::Right>({}, right, outputSize, currentFrame, ofile);
                            break;
                        case MultiviewOutputStrategy::Separate:
                            writeOutMultiview<MultiviewOutputStrategy::Separate>(left, right, outputSize, currentFrame, ofile);
                            break;
                        case MultiviewOutputStrategy::Merged:
                            writeOutMultiview<MultiviewOutputStrategy::Merged>(left, right, outputSize, currentFrame, ofile);
                            break;
                        }
                    } else if (images.count() == 1) {
                        // this is either the left or right eye content (2D texture)
                        if (ofile.isEmpty())
                            ofile = generateOfName<MultiviewOutputStrategy::Single>(currentFrame);
                        writeOutGrabbedFrame(images[0], outputSize, ofile);
                    }
                }

                if (viewCount >= 2) {
                    currentFrame += 1;
                } else {
                    currentSingleViewEye = (currentSingleViewEye + 1) % 2;
                    if (currentSingleViewEye == 0)
                        currentFrame += 1;
                }

                if (exitAfter > 0 && currentFrame > exitAfter)
                    QCoreApplication::exit();

            }, Qt::DirectConnection);
        }
    }

    return app.exec();
}

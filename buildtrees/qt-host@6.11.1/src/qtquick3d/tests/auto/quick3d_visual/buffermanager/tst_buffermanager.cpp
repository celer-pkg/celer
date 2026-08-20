// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QQuickView>

#include <private/qquick3dviewport_p.h>
#include <ssg/qssgrendercontextcore.h>
#include <private/qssgrenderbuffermanager_p.h>
#include <private/qquick3dresourceloader_p.h>

#if QT_CONFIG(vulkan)
#include <QVulkanInstance>
#endif

#include <QThread>

#include "../shared/util.h"

static inline void renderNextFrame(QQuick3DTestOffscreenRenderer *renderer, bool *readCompleted, QRhiReadbackResult *readResult, QImage *result)
{
    renderer->qmlEngine->collectGarbage();
    QGuiApplication::processEvents();
    renderer->renderControl->polishItems();
    renderer->renderControl->beginFrame();
    renderer->renderControl->sync();
    renderer->renderControl->render();
    renderer->enqueueReadback(readCompleted, readResult, result);
    renderer->renderControl->endFrame();
}

class tst_BufferManager : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void staticScene_data();
    void staticScene();
    void dynamicScene();

private:
    bool initRenderer(QQuick3DTestOffscreenRenderer *renderer, const QString &filename);
#if QT_CONFIG(vulkan)
    QVulkanInstance vulkanInstance;
#endif
};

void tst_BufferManager::initTestCase()
{
    QQuick3DDataTest::initTestCase();
    if (!initialized())
        return;

#if QT_CONFIG(vulkan)
    vulkanInstance.setLayers({ "VK_LAYER_LUNARG_standard_validation" });
    vulkanInstance.create(); // may fail, which is fine is Vulkan is not used in the first place
#endif
}

void tst_BufferManager::staticScene_data()
{
    QTest::addColumn<QString>("qmlSource");
    QTest::addColumn<int>("pathTextureCount");
    QTest::addColumn<int>("sgTextureCount");
    QTest::addColumn<int>("customTextureCount");
    QTest::addColumn<int>("meshCount");
    QTest::addColumn<int>("customMeshCount");

    QTest::newRow("empty scene") << "empty.qml" << 0 << 0 << 0 << 0 << 0;

    // Path Meshes
    QTest::newRow("unshared path meshes") << "pathedMeshesUnshared.qml" << 0 << 0 << 0 << 4 << 0;
    QTest::newRow("shared path meshes") << "pathedMeshesShared.qml" << 0 << 0 << 0 << 4 << 0;

    // Custom Meshes
    QTest::newRow("unused Custom Geometry") << "customGeometryUnused.qml" << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("unshared Custom Geometry") << "customGeometryUnshared.qml" << 0 << 0 << 0 << 0 << 3;
    QTest::newRow("shared Custom Geometry") << "customGeometryShared.qml" << 0 << 0 << 0 << 0 << 3;

    // Path Textures
    QTest::newRow("unused path textures") << "pathedTexturesUnused.qml" << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("unshared path textures") << "pathedTexturesUnshared.qml" << 5 << 0 << 0 << 1 << 0;
    QTest::newRow("shared path textures") << "pathedTexturesShared.qml" << 3 << 0 << 0 << 1 << 0;
    QTest::newRow("mipmodes path textures") << "pathedTexturesMipModes.qml" << 3 << 0 << 0 << 1 << 0;

    // SG (Quick) Textures
    QTest::newRow("unused SG textures") << "sgTexturesUnused.qml" << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("unshared SG textures") << "sgTexturesUnshared.qml" << 0 << 4 << 0 << 1 << 0;
    QTest::newRow("shared SG textures") << "sgTexturesShared.qml" << 0 << 3 << 0 << 1 << 0;

    // Custom Textures
    QTest::newRow("unused Custom Textures") << "customTexturesUnused.qml" << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("unshared Custom Textures") << "customTexturesUnshared.qml" << 0 << 0 << 3 << 1 << 0;
    QTest::newRow("unshared Custom Textures") << "customTexturesShared.qml" << 0 << 0 << 2 << 1 << 0;

    // ResourceLoader
    QTest::newRow("only ResourceLoader") << "resourceLoaderOnly.qml" << 3 << 1 << 1 << 4 << 3;
    QTest::newRow("mixed ResourceLoader") << "resourceLoaderMixed.qml" << 5 << 1 << 2 << 5 << 4;
    QTest::newRow("multiple ResourceLoader") << "resourceLoaderMultiple.qml" << 3 << 1 << 2 << 5 << 4;
}

void tst_BufferManager::staticScene()
{
    QFETCH(QString, qmlSource);
    QFETCH(int, pathTextureCount);
    QFETCH(int, sgTextureCount);
    QFETCH(int, customTextureCount);
    QFETCH(int, meshCount);
    QFETCH(int, customMeshCount);

    QQuick3DTestOffscreenRenderer renderer;
    QVERIFY(initRenderer(&renderer, qmlSource));

    if (renderer.quickWindow->rendererInterface()->graphicsApi() == QSGRendererInterface::OpenGL) {
#ifdef Q_OS_MACOS
        QSKIP("Skipping test due to sofware OpenGL renderer problems on macOS");
#endif
    }

    bool readCompleted = false;
    QRhiReadbackResult readResult;
    QImage result;

    renderNextFrame(&renderer, &readCompleted, &readResult, &result);

    const auto &context = QQuick3DSceneManager::getOrSetWindowAttachment(*renderer.quickWindow)->rci();
    QVERIFY(context);

    const auto &bufferManager = context->bufferManager();

    QCOMPARE(bufferManager->getImageMap().size(), pathTextureCount);
    QCOMPARE(bufferManager->getSGImageMap().size(), sgTextureCount);
    QCOMPARE(bufferManager->getCustomTextureMap().size(), customTextureCount);
    QCOMPARE(bufferManager->getMeshMap().size(), meshCount);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), customMeshCount);
}

void tst_BufferManager::dynamicScene()
{
    QQuick3DTestOffscreenRenderer renderer;
    QVERIFY(initRenderer(&renderer, QString("dynamic.qml")));

    if (renderer.quickWindow->rendererInterface()->graphicsApi() == QSGRendererInterface::OpenGL) {
#ifdef Q_OS_MACOS
        QSKIP("Skipping test due to sofware OpenGL renderer problems on macOS");
#endif
    }

    bool readCompleted = false;
    QRhiReadbackResult readResult;
    QImage result;

    renderNextFrame(&renderer, &readCompleted, &readResult, &result);

    const auto &context = QQuick3DSceneManager::getOrSetWindowAttachment(*renderer.quickWindow)->rci();
    QVERIFY(context);

    const auto &bufferManager = context->bufferManager();

    // Check for the initial state (1 static path model)
    QCOMPARE(bufferManager->getImageMap().size(), 0);
    QCOMPARE(bufferManager->getSGImageMap().size(), 0);
    QCOMPARE(bufferManager->getCustomTextureMap().size(), 0);
    QCOMPARE(bufferManager->getMeshMap().size(), 1);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 0);

    // Get the test controller object
    const auto controller = renderer.rootItem->property("controller").value<QQuick3DNode*>();
    QVERIFY(controller);

    auto addModel = [controller](const QString &path) -> QQuick3DModel* {
        QQuick3DModel *model = nullptr;
        QMetaObject::invokeMethod(controller, "addModel", Q_RETURN_ARG(QQuick3DModel*, model), Q_ARG(QString, path));
        return model;
    };

    auto addDynamicModel = [controller]() -> QQuick3DModel* {
        QQuick3DModel *model = nullptr;
        QMetaObject::invokeMethod(controller, "addDynamicModel", Q_RETURN_ARG(QQuick3DModel*, model));
        return model;
    };

    auto addTexture = [controller](const QString &path) -> QQuick3DTexture* {
        QQuick3DTexture *texture = nullptr;
        QMetaObject::invokeMethod(controller, "addTexture", Q_RETURN_ARG(QQuick3DTexture*, texture), Q_ARG(QString, path));
        return texture;
    };

    auto addDynamicTexture = [controller]() -> QQuick3DTexture* {
        QQuick3DTexture *texture = nullptr;
        QMetaObject::invokeMethod(controller, "addDynamicTexture", Q_RETURN_ARG(QQuick3DTexture*, texture));
        return texture;
    };

    auto addQmlTexture = [controller]() -> QQuick3DModel* {
        QQuick3DModel *model = nullptr;
        QMetaObject::invokeMethod(controller, "addQmlTexture", Q_RETURN_ARG(QQuick3DModel*, model));
        return model;
    };

    auto addQmlSharedTexture = [controller]() -> QQuick3DModel* {
        QQuick3DModel *model = nullptr;
        QMetaObject::invokeMethod(controller, "addQmlSharedTexture", Q_RETURN_ARG(QQuick3DModel*, model));
        return model;
    };

    auto addResourceLoader = [controller]() -> QQuick3DResourceLoader* {
        QQuick3DResourceLoader *loader = nullptr;
        QMetaObject::invokeMethod(controller, "addResourceLoader", Q_RETURN_ARG(QQuick3DResourceLoader*, loader));
        return loader;
    };

    // models (path)
    // add another sphere (count should stay 1)
    auto sphereModel = addModel("#Sphere");
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getMeshMap().size(), 1);
    // add a cube (count == 2)
    QQuick3DModel *model = addModel("#Cube");
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getMeshMap().size(), 2);
    // remove the cube model (count == 1)
    QMetaObject::invokeMethod(controller, "removeModel", Qt::DirectConnection);
    delete model;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getMeshMap().size(), 1);
    QMetaObject::invokeMethod(controller, "removeModel");
    delete sphereModel;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getMeshMap().size(), 1);

    // models (customGeometry)
    auto dynamicModel = addDynamicModel();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 1);
    auto dynamicModel2 = addDynamicModel();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 2);

    QMetaObject::invokeMethod(controller, "removeDynamicModel");
    delete dynamicModel2;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 1);

    QMetaObject::invokeMethod(controller, "removeDynamicModel");
    delete dynamicModel;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 0);


    // textures (path)
    QCOMPARE(bufferManager->getImageMap().size(), 0);
    auto texture = addTexture("noise1.jpg");
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getImageMap().size(), 1);
    QMetaObject::invokeMethod(controller, "removeTexture");
    delete texture;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getImageMap().size(), 0);

    // textures (custom)
    QCOMPARE(bufferManager->getCustomTextureMap().size(), 0);
    auto dynamicTexture = addDynamicTexture();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getCustomTextureMap().size(), 1);
    QMetaObject::invokeMethod(controller, "removeDynamicTexture");
    delete dynamicTexture;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getCustomTextureMap().size(), 0);

    // textures (sg)
    QCOMPARE(bufferManager->getSGImageMap().size(), 0);
    auto qmlTexture = addQmlTexture();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 1);
    auto qmlTexture2 = addQmlTexture();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 2);
    QMetaObject::invokeMethod(controller, "removeQmlTexture");
    delete qmlTexture2;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 1);
    QMetaObject::invokeMethod(controller, "removeQmlTexture");
    delete qmlTexture;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 0);

    QCOMPARE(bufferManager->getSGImageMap().size(), 0);
    auto qmlSharedTexture = addQmlSharedTexture();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 1);
    auto qmlSharedTexture2 = addQmlSharedTexture();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 1);
    QMetaObject::invokeMethod(controller, "removeQmlSharedTexture");
    delete qmlSharedTexture2;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 1);
    QMetaObject::invokeMethod(controller, "removeQmlSharedTexture");
    delete qmlSharedTexture;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getSGImageMap().size(), 0);

    // Make sure scene is in its initial state
    QCOMPARE(bufferManager->getImageMap().size(), 0);
    QCOMPARE(bufferManager->getSGImageMap().size(), 0);
    QCOMPARE(bufferManager->getCustomTextureMap().size(), 0);
    QCOMPARE(bufferManager->getMeshMap().size(), 1);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 0);

    // Add a ResourceLoader
    auto resourceLoader = addResourceLoader();
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getImageMap().size(), 3);
    QCOMPARE(bufferManager->getSGImageMap().size(), 1);
    QCOMPARE(bufferManager->getCustomTextureMap().size(), 1);
    QCOMPARE(bufferManager->getMeshMap().size(), 5);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 1);
    // Remove the ResourceLoader
    delete resourceLoader;
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QCOMPARE(bufferManager->getImageMap().size(), 0);
    QCOMPARE(bufferManager->getSGImageMap().size(), 0);
    QCOMPARE(bufferManager->getCustomTextureMap().size(), 0);
    QCOMPARE(bufferManager->getMeshMap().size(), 1);
    QCOMPARE(bufferManager->getCustomMeshMap().size(), 0);
}

bool tst_BufferManager::initRenderer(QQuick3DTestOffscreenRenderer *renderer, const QString &filename)
{
    const bool initSuccess = renderer->init(testFileUrl(filename),
#if QT_CONFIG(vulkan)
                                            &vulkanInstance
#else
                                            nullptr
#endif
    );
    return initSuccess;
}

QTEST_MAIN(tst_BufferManager)
#include "tst_buffermanager.moc"

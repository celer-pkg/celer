// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QQuickRenderTarget>
#include <QQuickGraphicsDevice>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlComponent>

#if QT_CONFIG(vulkan)
#include <QVulkanInstance>
#endif

#include "../shared/util.h"

class tst_RenderControl : public QQuick3DDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cube();
    void textureSourceItem();
    void dynamicLights();

private:
    bool initRenderer(QQuick3DTestOffscreenRenderer *renderer, const char *filename);
#if QT_CONFIG(vulkan)
    QVulkanInstance vulkanInstance;
#endif
};

void tst_RenderControl::initTestCase()
{
    QQuick3DDataTest::initTestCase();
    if (!initialized())
        return;

#if QT_CONFIG(vulkan)
    vulkanInstance.setLayers({ "VK_LAYER_LUNARG_standard_validation" });
    vulkanInstance.create(); // may fail, which is fine is Vulkan is not used in the first place
#endif
}

bool tst_RenderControl::initRenderer(QQuick3DTestOffscreenRenderer *renderer, const char *filename)
{
    const bool initSuccess = renderer->init(testFileUrl(QString::fromLatin1(filename)),
#if QT_CONFIG(vulkan)
                                     &vulkanInstance
#else
                                     nullptr
#endif
        );
    return initSuccess;
}

const int FUZZ = 5;

static inline void renderNextFrame(QQuick3DTestOffscreenRenderer *renderer,
                                   bool *readCompleted,
                                   QRhiReadbackResult *readResult,
                                   QImage *result)
{
    renderer->renderControl->polishItems();
    renderer->renderControl->beginFrame();
    renderer->renderControl->sync();
    renderer->renderControl->render();
    renderer->enqueueReadback(readCompleted, readResult, result);
    renderer->renderControl->endFrame();
}

void tst_RenderControl::cube()
{
    QQuick3DTestOffscreenRenderer renderer;
    QVERIFY(initRenderer(&renderer, "cube_with_size.qml"));

    bool readCompleted = false;
    QRhiReadbackResult readResult;
    QImage result;

    renderNextFrame(&renderer, &readCompleted, &readResult, &result);

    QVERIFY(readCompleted);
    QVERIFY(!result.isNull());
    QCOMPARE(result.size(), QSize(640, 480)); // no trouble with scale factors since it's all offscreen

    QVERIFY(comparePixel(result, 50, 50, 1, Qt::black, FUZZ));

    // front face of the cube is lighter gray-ish
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(239, 239, 239), FUZZ));

    // the top is darker
    QVERIFY(comparePixelNormPos(result, 0.5, 0.375, QColor::fromRgb(181, 181, 181), FUZZ));
}

void tst_RenderControl::textureSourceItem()
{
    QQuick3DTestOffscreenRenderer renderer;
    QVERIFY(initRenderer(&renderer, "sourceitem.qml"));

    // We do expect a perfect frame, meaning that the 2D content in
    // the textures must be rendered in the first frame and should not
    // be deferred. Unlike with an on-screen window, we can verify
    // this in a robust manner here because we are in full control of
    // when exactly a new frame is generated.

    bool readCompleted = false;
    QRhiReadbackResult readResult;
    QImage result;

    // Frame 1
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QVERIFY(readCompleted);
    QCOMPARE(result.size(), QSize(640, 480));

    // from left to right: red, green, blue
    QVERIFY(comparePixelNormPos(result, 0.275, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 128, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.5, QColor::fromRgb(0, 0, 255), FUZZ));

    result = QImage();

    // make a Texture.sourceItem refer to something else now
    QMetaObject::invokeMethod(renderer.rootItem, "makeThirdReferToStandaloneSourceItem");

    // Frame 2
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QVERIFY(readCompleted);
    QCOMPARE(result.size(), QSize(640, 480));

    // from left to right: red, green, red
    QVERIFY(comparePixelNormPos(result, 0.275, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 128, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));

    result = QImage();

    // exercise resizing the render target, the root item, and the scene
    QVERIFY(renderer.resize(QSize(1280, 960)));

    // Frame 3
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QVERIFY(readCompleted);
    QCOMPARE(result.size(), QSize(1280, 960));

    // from left to right: red, green, red (unchanged, compared to
    // frame 2, apart from the increased size, but the normalized
    // positions work as-is)
    QVERIFY(comparePixelNormPos(result, 0.275, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 128, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));

    result = QImage();

    // switch the third sphere to a different sourceItem, now an Item
    // tree that has an explicit layer.enabled: true on it (meaning no
    // implicit QSGLayer gets created internally on Quick3D side)
    QMetaObject::invokeMethod(renderer.rootItem, "makeThirdReferToExplicitLayerBasedSourceItem");

    // Frame 4
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QVERIFY(readCompleted);
    QCOMPARE(result.size(), QSize(1280, 960));

    // from left to right: red, green, gray
    QVERIFY(comparePixelNormPos(result, 0.275, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 128, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.5, QColor::fromRgb(128, 128, 128), FUZZ));

    result = QImage();

    // switch the third sphere to with an Image (which itself is a
    // textureProvider) as the sourceItem
    QMetaObject::invokeMethod(renderer.rootItem, "makeThirdReferToImageSourceItem");

    // Frame 5
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QVERIFY(readCompleted);
    QCOMPARE(result.size(), QSize(1280, 960));

    QVERIFY(comparePixelNormPos(result, 0.275, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 128, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.7, 0.5, QColor::fromRgb(64, 205, 82), FUZZ)); // green from Qt logo image
    QVERIFY(comparePixelNormPos(result, 0.725, 0.5, QColor::fromRgb(255, 255, 255), FUZZ)); // white from Qt logo image

    result = QImage();

    // switch the third sphere to use a sourceItem that has non-opaque content
    QMetaObject::invokeMethod(renderer.rootItem, "makeThirdReferToSemiTransparentSourceItem");

    // Frame 6
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QVERIFY(readCompleted);
    QCOMPARE(result.size(), QSize(1280, 960));

    // from left to right: red, green, transparent/blue
    QVERIFY(comparePixelNormPos(result, 0.275, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 128, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.44, QColor::fromRgb(0, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.46, QColor::fromRgb(0, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.5, QColor::fromRgb(0, 0, 255), FUZZ));

    result = QImage();

    // make the sourceItem's size half - this leads to the blue
    // rectangle occupy a larger area on the sphere's surface
    QMetaObject::invokeMethod(renderer.rootItem, "makeSemiTransparentSourceItemSmaller");

    // Frame 7
    renderNextFrame(&renderer, &readCompleted, &readResult, &result);
    QVERIFY(readCompleted);
    QCOMPARE(result.size(), QSize(1280, 960));

    QVERIFY(comparePixelNormPos(result, 0.275, 0.5, QColor::fromRgb(255, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 128, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.44, QColor::fromRgb(0, 0, 0), FUZZ));
    QVERIFY(comparePixelNormPos(result, 0.725, 0.46, QColor::fromRgb(0, 0, 255), FUZZ)); // different from previous frame
    QVERIFY(comparePixelNormPos(result, 0.725, 0.5, QColor::fromRgb(0, 0, 255), FUZZ));
}

static inline QObject *invokeAndGetObject(QObject *target, const char *func)
{
    QVariant result;
    QMetaObject::invokeMethod(target, func, Q_RETURN_ARG(QVariant, result));
    return result.value<QObject *>();
}

void tst_RenderControl::dynamicLights()
{
    QQuick3DTestOffscreenRenderer renderer;
    QVERIFY(initRenderer(&renderer, "dynamic_lights.qml"));

    if (renderer.quickWindow->rendererInterface()->graphicsApi() == QSGRendererInterface::OpenGL) {
#ifdef Q_OS_MACOS
        QSKIP("Skipping test due to sofware OpenGL renderer problems on macOS");
#endif
    }

#if defined(Q_OS_WIN)
#ifdef Q_PROCESSOR_ARM
    if (qgetenv("QTEST_ENVIRONMENT") == "ci") {
        QSKIP("Skipping test due to problems on windows arm64 on CI");
    }
#endif
#endif

    bool readCompleted = false;
    QRhiReadbackResult readResult;
    QImage result;
    QObject *directionalLight = nullptr;
    QObject *pointLight = nullptr;
    QObject *spotLight = nullptr;

    // Case: scene without lights
    {
        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: add a DirectionalLight with default property values
    {
        result = QImage();
        directionalLight = invokeAndGetObject(renderer.rootItem, "addDirectionalLight");

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        // front face of the cube is lighter gray-ish
        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(239, 239, 239), FUZZ));
        // the top is darker
        QVERIFY(comparePixelNormPos(result, 0.5, 0.375, QColor::fromRgb(181, 181, 181), FUZZ));
        // the "floor" is even darker
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(153, 153, 153), FUZZ));
    }

    // Case: destroy the DirectionalLight
    {
        result = QImage();
        delete directionalLight;

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: add a shadow casting DirectionalLight emitting downwards
    {
        result = QImage();
        directionalLight = invokeAndGetObject(renderer.rootItem, "addShadowCastingDirectionalLight");

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        // front face of the cube is black
        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 0, 0), FUZZ));
        // the top is bright
        QVERIFY(comparePixelNormPos(result, 0.5, 0.375, QColor::fromRgb(234, 234, 234), FUZZ));
        // the "floor" is even brighter
        QVERIFY(comparePixelNormPos(result, 0.5, 0.9, QColor::fromRgb(248, 248, 248), FUZZ));
        // except where it's in shadow
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: toggle 'visible'
    {
        result = QImage();
        directionalLight->setProperty("visible", false);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 0, 0), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.375, QColor::fromRgb(0, 0, 0), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.9, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: move the camera up and look downwards a bit, add a (non-shadow casting) full-green PointLight
    {
        result = QImage();
        QMetaObject::invokeMethod(renderer.rootItem, "moveCamera");
        pointLight = invokeAndGetObject(renderer.rootItem, "addPointLight");

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        // top of the cube is very green
        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 255, 0), FUZZ));
        // the front of the cube is now black
        QVERIFY(comparePixelNormPos(result, 0.5, 0.666, QColor::fromRgb(0, 0, 0), FUZZ));
        // floor is darker green
        QVERIFY(comparePixelNormPos(result, 0.5, 0.9, QColor::fromRgb(0, 140, 0), FUZZ));
    }

    // Case: make the PointLight cast shadows
    {
        result = QImage();
        pointLight->setProperty("castsShadow", true);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 255, 0), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.666, QColor::fromRgb(0, 0, 0), FUZZ));
        // floor is darker due to the shadow
        QVERIFY(comparePixelNormPos(result, 0.5, 0.9, QColor::fromRgb(0, 72, 0), FUZZ));
    }

    // Case: make the DirectionalLight visible and cast shadows
    {
        result = QImage();
        directionalLight->setProperty("visible", true);
        directionalLight->setProperty("castsShadow", true);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 255, 255), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.9, QColor::fromRgb(248, 255, 248), FUZZ));
        // floor in shadow, now with some green added
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 82, 0), FUZZ));
    }

    // Case: make the DirectionalLight not cast shadows and reduce brightness
    {
        result = QImage();
        directionalLight->setProperty("castsShadow", false);
        directionalLight->setProperty("brightness", 0.2);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 255, 255), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(173, 188, 173), FUZZ));
    }

    // Case: destroy the DirectionalLight
    {
        result = QImage();
        delete directionalLight;

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 255, 0), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.666, QColor::fromRgb(0, 0, 0), FUZZ));
        // floor is darker due to the shadow
        QVERIFY(comparePixelNormPos(result, 0.5, 0.9, QColor::fromRgb(0, 72, 0), FUZZ));
    }

    // Case: re-add a shadow casting DirectionalLight emitting downwards
    {
        result = QImage();
        directionalLight = invokeAndGetObject(renderer.rootItem, "addShadowCastingDirectionalLight");

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 255, 255), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.9, QColor::fromRgb(248, 255, 248), FUZZ));
        // floor in shadow, now with some green added
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 82, 0), FUZZ));
    }

    // Case: destroy the PointLight
    {
        result = QImage();
        delete pointLight;

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 255, 255), FUZZ));
        // floor in shadow
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: destroy the DirectionalLight
    {
        result = QImage();
        delete directionalLight;

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // With the default forward renderer, we can have up to 4 directional lights
    // and up to 16 positional lights (point/spot) at the same time. Each light
    // can cast shadows as well.  If a scene has more directional lights, only the
    // first 4 defined will be used for lighting. The same goes for point/spot lights,
    // though there is some expectation in the future we will determine the 16 closest
    // positional lights to the object being drawn.

    // Case: add 10 shadow casting DirectionalLights (note that only 4 of these
    // will actually do anything)
    QVarLengthArray<QObject *, 10> directionalLights;
    {
        result = QImage();
        for (int i = 0; i < 10; ++i)
            directionalLights.append(invokeAndGetObject(renderer.rootItem, "addShadowCastingDirectionalLight"));

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 255, 255), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: make half of the lights 32 bit
    {
        for (int i = 0; i < 10; i += 2)
            directionalLights[i]->setProperty("use32BitShadowmap", true);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 255, 255), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: make some invisible and destroy some
    {
        result = QImage();
        delete directionalLights[2];
        directionalLights.remove(2);
        delete directionalLights[3];
        directionalLights.remove(3);
        directionalLights[0]->setProperty("visible", false);
        directionalLights[2]->setProperty("visible", false);
        directionalLights[4]->setProperty("visible", false);
        directionalLights[6]->setProperty("visible", false);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 0, 0), FUZZ));
    }

    // Case: destroy one more DirectionalLight and add a shadow casting PointLight
    {
        result = QImage();
        QVERIFY(directionalLights.last()->property("visible").toBool());
        delete directionalLights.last();
        directionalLights.removeLast();
        pointLight = invokeAndGetObject(renderer.rootItem, "addPointLight");
        pointLight->setProperty("castsShadow", true);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(255, 255, 255), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(0, 82, 0), FUZZ));
    }

    // Case: delete all but one DirectionalLight, reduce its color and brightness, and add a red SpotLight
    {
        result = QImage();
        for (int i = 1; i < directionalLights.size(); i++)
            delete directionalLights[i];
        directionalLights[0]->setProperty("visible", true);
        directionalLights[0]->setProperty("brightness", 0.1);
        directionalLights[0]->setProperty("color", QColor(Qt::blue));
        spotLight = invokeAndGetObject(renderer.rootItem, "addSpotLight");
        spotLight->setProperty("castsShadow", true);

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 255, 255), FUZZ));
        QVERIFY(comparePixelNormPos(result, 0.5, 0.8, QColor::fromRgb(10, 82, 0), FUZZ));
    }

    // Case: add a lot of lights, to exceed the limit of 15. Should survive
    // gracefully, ignoring the extra lights.
    {
        result = QImage();
        for (int i = 0; i < 30; ++i)
            invokeAndGetObject(renderer.rootItem, "addPointLight");

        renderNextFrame(&renderer, &readCompleted, &readResult, &result);
        QVERIFY(readCompleted);
        QCOMPARE(result.size(), QSize(640, 480));

        QVERIFY(comparePixelNormPos(result, 0.5, 0.5, QColor::fromRgb(0, 255, 0), FUZZ));
    }
}

QTEST_MAIN(tst_RenderControl)
#include "tst_rendercontrol.moc"

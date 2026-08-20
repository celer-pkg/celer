// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "util.h"

#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>

#include <QtQuick3D/qquick3d.h>

#if QT_CONFIG(vulkan)
#include <QVulkanInstance>
#endif

QQuick3DDataTest *QQuick3DDataTest::m_instance = 0;

QQuick3DDataTest::QQuick3DDataTest() :
    m_initialized(false),
#ifdef QT_TESTCASE_BUILDDIR
    m_dataDirectory(QTest::qFindTestData("data", QT_QMLTEST_DATADIR, 0, QT_TESTCASE_BUILDDIR)),
#else
    m_dataDirectory(QTest::qFindTestData("data", QT_QMLTEST_DATADIR, 0)),
#endif

    m_dataDirectoryUrl(m_dataDirectory.startsWith(QLatin1Char(':'))
        ? QUrl(QLatin1String("qrc") + m_dataDirectory)
        : QUrl::fromLocalFile(m_dataDirectory + QLatin1Char('/')))
{
    m_instance = this;
}

QQuick3DDataTest::~QQuick3DDataTest()
{
    m_instance = 0;
}

void QQuick3DDataTest::initTestCase()
{
    QVERIFY2(!m_dataDirectory.isEmpty(), "'data' directory not found");
    m_directory = QFileInfo(m_dataDirectory).absolutePath();
    if (m_dataDirectoryUrl.scheme() != QLatin1String("qrc"))
        QVERIFY2(QDir::setCurrent(m_directory), qPrintable(QLatin1String("Could not chdir to ") + m_directory));

    if (QGuiApplication::platformName() == QLatin1String("offscreen")
        || QGuiApplication::platformName() == QLatin1String("minimal"))
    {
        QSKIP("Skipping visual 3D tests due to running with offscreen/minimal");
    }

    if (!isRunningOnRhi())
        QSKIP("Skipping visual 3D tests due to not running with QRhi");

#if QT_CONFIG(opengl)
    QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat());
#endif

    m_initialized = true;
}

void QQuick3DDataTest::cleanupTestCase()
{
    m_initialized = false;
}

QString QQuick3DDataTest::testFile(const QString &fileName) const
{
    if (m_directory.isEmpty())
        qFatal("QQuick3DDataTest::initTestCase() not called.");
    QString result = m_dataDirectory;
    result += QLatin1Char('/');
    result += fileName;
    return result;
}

bool QQuick3DDataTest::isRunningOnRhi()
{
    static bool retval = false;
    static bool decided = false;
    if (!decided) {
        decided = true;
        QQuickView dummy;
        dummy.show();
        if (!QTest::qWaitForWindowExposed(&dummy)) {
            [](){ QFAIL("Could not show a QQuickView"); }();
            return false;
        }
        QSGRendererInterface::GraphicsApi api = dummy.rendererInterface()->graphicsApi();
        retval = QSGRendererInterface::isApiRhiBased(api);
        dummy.hide();
    }
    return retval;
}

QQuickView *QQuick3DDataTest::createView(const QString &filename, const QSize &windowSize)
{
    QQuickView *view = new QQuickView;
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->resize(windowSize);
    view->setSource(testFileUrl(filename));
    view->show();
    return view;
}

QQuickWindow *QQuick3DDataTest::createWindow(const QString &filename, const QSize &windowSize)
{
    QQuickWindow *window = new QQuickWindow;
    window->setWidth(windowSize.width());
    window->setHeight(windowSize.height());

    QQmlEngine engine;
    QQmlComponent component(&engine, testFileUrl(filename));
    if (component.isError()) {
        for (const QQmlError &error : component.errors())
            qWarning() << error.url() << error.line() << error;
        delete window;
        return nullptr;
    }

    QObject *rootObject = component.create();
    if (component.isError()) {
        for (const QQmlError &error : component.errors())
            qWarning() << error.url() << error.line() << error;
        delete window;
        return nullptr;
    }

    QQuickItem *rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!rootItem) {
        qWarning("No root item");
        delete window;
        return nullptr;
    }

    window->contentItem()->setSize(rootItem->size());
    rootItem->setParentItem(window->contentItem());

    window->show();
    return window;
}

static inline void saveImageIfEnabled(const QImage &image)
{
    if (qEnvironmentVariableIntValue("QT_QUICK3D_TEST_DEBUG")) {
        static int cnt = 1;
        const QString fn = QString::asprintf("quick3d_test_%d.png", cnt);
        ++cnt;
        image.save(fn);
        qDebug("grab result saved to %s", qPrintable(fn));
    }
}

QImage QQuick3DDataTest::grab(QQuickWindow *window)
{
    const qreal dpr = window->devicePixelRatio();
    QImage content = window->grabWindow();
    if (content.isNull()) {
        [](){ QFAIL("No QImage received from grabWindow"); }();
        return QImage();
    }
    const QSize expectedSize = window->size() * dpr;
    if (content.size() != expectedSize) {
        qDebug() << "grabbed image size" << content.size() << "expected size" << expectedSize;
        [](){ QFAIL("Unexpected QImage size from grabWindow"); }();
        return QImage();
    }
    saveImageIfEnabled(content);
    return content;
}

static inline bool compareColor(int value, int expected, int fuzz, int debugX, int debugY, const char *debugChannelName)
{
    const int diff = qAbs(value - expected);
    if (diff > fuzz) {
        qDebug("%s channel mismatch at position (%d, %d). Expected %d with maximum difference %d, got %d.",
               debugChannelName, debugX, debugY, expected, fuzz, value);
        return false;
    }
    return true;
}

static inline bool compareColorAt(const QImage &image, int physicalX, int physicalY, const QColor &expected, int fuzz)
{
    const QRgb c = image.pixel(physicalX, physicalY);
    if (!compareColor(qRed(c), expected.red(), fuzz, physicalX, physicalY, "Red"))
        return false;
    if (!compareColor(qGreen(c), expected.green(), fuzz, physicalX, physicalY, "Green"))
        return false;
    if (!compareColor(qBlue(c), expected.blue(), fuzz, physicalX, physicalY, "Blue"))
        return false;
    if (!compareColor(qAlpha(c), expected.alpha(), fuzz, physicalX, physicalY, "Alpha"))
        return false;
    return true;
}

bool QQuick3DDataTest::comparePixel(const QImage &image, int logicalX, int logicalY, qreal dpr, const QColor &expected, int fuzz)
{
    const int physicalX = qCeil(logicalX * dpr);
    const int physicalY = qCeil(logicalY * dpr);
    return compareColorAt(image, physicalX, physicalY, expected, fuzz);
}

bool QQuick3DDataTest::comparePixelNormPos(const QImage &image, qreal normalizedX, qreal normalizedY, const QColor &expected, int fuzz)
{
    const int physicalWidth = image.width();
    const int physicalHeight = image.height();
    const int physicalX = qCeil(physicalWidth * normalizedX);
    const int physicalY = qCeil(physicalHeight * normalizedY);
    return compareColorAt(image, physicalX, physicalY, expected, fuzz);
}

bool QQuick3DTestOffscreenRenderer::init(const QUrl &fileUrl, void *vulkanInstance)
{
    renderControl.reset(new QQuickRenderControl);
    quickWindow.reset(new QQuickWindow(renderControl.data()));
#if QT_CONFIG(vulkan)
    // We don't know if Vulkan is used on a given platform since in this test we just go
    // with whatever platform defaults Qt Quick chooses, but be prepared and have the
    // instance available if needed.
    QVulkanInstance *inst = static_cast<QVulkanInstance *>(vulkanInstance);
    if (inst->isValid())
        quickWindow->setVulkanInstance(inst);
#else
    Q_UNUSED(vulkanInstance);
#endif

    qmlEngine.reset(new QQmlEngine);
    qmlComponent.reset(new QQmlComponent(qmlEngine.data(), fileUrl));
    if (qmlComponent->isLoading()) {
        qWarning("Component's isLoading() is unexpectedly true");
        return false;
    }

    if (qmlComponent->isError()) {
        for (const QQmlError &error : qmlComponent->errors())
            qWarning() << error.url() << error.line() << error;
        return false;
    }

    QObject *rootObject = qmlComponent->create();
    if (qmlComponent->isError()) {
        for (const QQmlError &error : qmlComponent->errors())
            qWarning() << error.url() << error.line() << error;
        return false;
    }

    rootItem = qobject_cast<QQuickItem *>(rootObject);
    if (!rootItem) {
        qWarning("No root item");
        return false;
    }

    quickWindow->contentItem()->setSize(rootItem->size());
    quickWindow->setGeometry(0, 0, rootItem->width(), rootItem->height());
    rootItem->setParentItem(quickWindow->contentItem());

    if (!renderControl->initialize()) {
        qWarning("RenderControl failed to initialized");
        return false;
    }

    QQuickRenderControlPrivate *rd = QQuickRenderControlPrivate::get(renderControl.data());
    rhi = rd->rhi;
    if (!rhi) {
        qWarning("RenderControl created no QRhi");
        return false;
    }

    const QSize size = rootItem->size().toSize();
    tex.reset(rhi->newTexture(QRhiTexture::RGBA8, size, 1,
                              QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
    if (!tex->create()) {
        qWarning("Failed to create QRhiTexture");
        return false;
    }

    ds.reset(rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, size, 1));
    if (!ds->create()) {
        qWarning("Failed to create depth-stencil buffer");
        return false;
    }

    QRhiTextureRenderTargetDescription rtDesc(QRhiColorAttachment(tex.data()));
    rtDesc.setDepthStencilBuffer(ds.data());
    texRt.reset(rhi->newTextureRenderTarget(rtDesc));
    rp.reset(texRt->newCompatibleRenderPassDescriptor());
    texRt->setRenderPassDescriptor(rp.data());
    if (!texRt->create()) {
        qWarning("Failed to create rendertarget");
        return false;
    }

    quickWindow->setRenderTarget(QQuickRenderTarget::fromRhiRenderTarget(texRt.data()));

    return true;
}

void QQuick3DTestOffscreenRenderer::enqueueReadback(bool *readCompleted, QRhiReadbackResult *readResult, QImage *result)
{
    *readCompleted = false;
    readResult->completed = [this, readCompleted, readResult, result] {
        *readCompleted = true;
        QImage wrapperImage(reinterpret_cast<const uchar *>(readResult->data.constData()),
            readResult->pixelSize.width(), readResult->pixelSize.height(),
            QImage::Format_RGBA8888_Premultiplied);
        if (rhi->isYUpInFramebuffer())
            *result = wrapperImage.flipped();
        else
            *result = wrapperImage.copy();
        saveImageIfEnabled(*result);
    };
    QRhiResourceUpdateBatch *readbackBatch = rhi->nextResourceUpdateBatch();
    readbackBatch->readBackTexture(tex.data(), readResult);
    activeCommandBuffer()->resourceUpdate(readbackBatch);
}

bool QQuick3DTestOffscreenRenderer::resize(const QSize &newSize)
{
    tex->setPixelSize(newSize);
    if (!tex->create()) {
        qWarning("Failed to recreate QRhiTexture");
        return false;
    }
    ds->setPixelSize(newSize);
    if (!ds->create()) {
        qWarning("Failed to recreate depth-stencil buffer");
        return false;
    }

    // Intentionally no texRt->create() or quickWindow->setRenderTarget(). It must work without those.

    rootItem->setSize(newSize);
    quickWindow->contentItem()->setSize(newSize);
    quickWindow->setGeometry(0, 0, newSize.width(), newSize.height());

    return true;
}

Q_GLOBAL_STATIC(QMutex, qQmlTestMessageHandlerMutex)

QQuick3DTestMessageHandler *QQuick3DTestMessageHandler::m_instance = 0;

void QQuick3DTestMessageHandler::messageHandler(QtMsgType, const QMessageLogContext &context, const QString &message)
{
    QMutexLocker locker(qQmlTestMessageHandlerMutex());
    if (QQuick3DTestMessageHandler::m_instance) {
        if (QQuick3DTestMessageHandler::m_instance->m_includeCategories)
            QQuick3DTestMessageHandler::m_instance->m_messages.push_back(QString("%1: %2").arg(context.category, message));
        else
            QQuick3DTestMessageHandler::m_instance->m_messages.push_back(message);
    }
}

QQuick3DTestMessageHandler::QQuick3DTestMessageHandler()
{
    QMutexLocker locker(qQmlTestMessageHandlerMutex());
    Q_ASSERT(!QQuick3DTestMessageHandler::m_instance);
    QQuick3DTestMessageHandler::m_instance = this;
    m_oldHandler = qInstallMessageHandler(messageHandler);
    m_includeCategories = false;
}

QQuick3DTestMessageHandler::~QQuick3DTestMessageHandler()
{
    QMutexLocker locker(qQmlTestMessageHandlerMutex());
    Q_ASSERT(QQuick3DTestMessageHandler::m_instance);
    qInstallMessageHandler(m_oldHandler);
    QQuick3DTestMessageHandler::m_instance = 0;
}

// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#include <QSignalSpy>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QtQuick3D/qquick3d.h>
#include <QtQml/private/qqmlabstractprofileradapter_p.h>
#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DProfilerAdapter : public QQmlAbstractProfilerAdapter
{
    Q_OBJECT
public:
    QQuick3DProfilerAdapter(QObject *parent = 0)
        : QQmlAbstractProfilerAdapter(parent)
    {
        QQuick3DProfiler::initialize(this);
        connect(this, &QQmlAbstractProfilerAdapter::profilingEnabled,
                QQuick3DProfiler::s_instance, &QQuick3DProfiler::startProfilingImpl, Qt::DirectConnection);
        connect(this, &QQmlAbstractProfilerAdapter::profilingEnabledWhileWaiting,
                QQuick3DProfiler::s_instance, &QQuick3DProfiler::startProfilingImpl, Qt::DirectConnection);
        connect(this, &QQmlAbstractProfilerAdapter::referenceTimeKnown,
                QQuick3DProfiler::s_instance, &QQuick3DProfiler::setTimer, Qt::DirectConnection);
        connect(this, &QQmlAbstractProfilerAdapter::profilingDisabled,
                QQuick3DProfiler::s_instance, &QQuick3DProfiler::stopProfilingImpl, Qt::DirectConnection);
        connect(this, &QQmlAbstractProfilerAdapter::profilingDisabledWhileWaiting,
                QQuick3DProfiler::s_instance, &QQuick3DProfiler::stopProfilingImpl, Qt::DirectConnection);
        connect(this, &QQmlAbstractProfilerAdapter::dataRequested,
                QQuick3DProfiler::s_instance, &QQuick3DProfiler::reportDataImpl, Qt::DirectConnection);
        connect(QQuick3DProfiler::s_instance, &QQuick3DProfiler::dataReady,
                this, &QQuick3DProfilerAdapter::receiveData, Qt::DirectConnection);
    }

    qint64 sendMessages(qint64 until, QList<QByteArray> &messages) override
    {
        Q_UNUSED(until);
        Q_UNUSED(messages);
        return 0;
    }
    void receiveData(const QVector<QQuick3DProfilerData> &new_data, const QHash<int, QByteArray> &eventData)
    {
        if (m_data.isEmpty())
            m_data = new_data;
        else
            m_data.append(new_data);
        m_eventData = eventData;
    }

    QVector<QQuick3DProfilerData> m_data;
    QHash<int, QByteArray> m_eventData;
};

QT_END_NAMESPACE

class tst_QQuick3DProfiler : public QQmlDataTest
{
    Q_OBJECT

    static constexpr int ID_MARKER = 0xed000000;
    static constexpr int ID_MARKER_MASK = 0xff000000;
    static constexpr int ID_MASK = 0x00ffffff;

    inline bool checkId(int id) const
    {
        return ((id & ID_MARKER_MASK) == ID_MARKER) && (id&ID_MASK) > 0;
    }

    bool isRunningOnRhi()
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
public:
    tst_QQuick3DProfiler();
private slots:
    void initTestCase() override;
    void testProfiling();
};

tst_QQuick3DProfiler::tst_QQuick3DProfiler()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
}

void tst_QQuick3DProfiler::initTestCase()
{
    QQmlDataTest::initTestCase();

    if (!isRunningOnRhi())
        QSKIP("Skipping quick3d profiler tests due to not running with QRhi");

#if QT_CONFIG(opengl)
    QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat());
#endif
}

void tst_QQuick3DProfiler::testProfiling()
{
    QQuick3DProfilerAdapter adapter;
    adapter.startProfiling(0xffffffff);
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.loadUrl(testFileUrl("scene.qml"));
        QObject *obj = component.create();
        QScopedPointer<QObject> cleanup(obj);
        QVERIFY(obj);
        QQuickWindow *window = qobject_cast<QQuickWindow*>(obj);
        QVERIFY(window);
        QVERIFY(QTest::qWaitForWindowExposed(window));
        QTest::qWait(2000);
        adapter.stopProfiling();
        adapter.reportData();
    }
    qint64 prevMeshSize = 0;
    qint64 prevTextureSize = 0;
    QVERIFY(!adapter.m_data.isEmpty());
    bool messageReceived[QQuick3DProfiler::MaximumQuick3DFrameType] = {false};
    for (const auto &data : std::as_const(adapter.m_data)) {
        QVERIFY(data.messageType == QQuick3DProfiler::Quick3DFrame);
        QVERIFY(data.time > 0);
        if (data.detailType < QQuick3DProfiler::MaximumQuick3DFrameType)
            messageReceived[data.detailType] = true;
        switch (data.detailType) {
        case QQuick3DProfiler::Quick3DRenderFrame:
        case QQuick3DProfiler::Quick3DSynchronizeFrame:
        case QQuick3DProfiler::Quick3DPrepareFrame:
            QVERIFY2(checkId(data.ids[0]), "view3d id is not valid");
            QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing profiling id data");
            QVERIFY2(data.subdata1 != 0, "frame is missing payload");
            break;
        case QQuick3DProfiler::Quick3DMeshLoad:
            if (data.subdata1 > prevMeshSize) {
                QVERIFY2(checkId(data.ids[0]), "mesh load id is not valid");
                QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing mesh load url");
            }
            prevMeshSize = data.subdata1;
            break;
        case QQuick3DProfiler::Quick3DCustomMeshLoad:
            QVERIFY2(checkId(data.ids[0]), "custom mesh load id is not valid");
            QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing profiling id data");
            break;
        case QQuick3DProfiler::Quick3DTextureLoad:
            if (data.subdata1 > prevTextureSize) {
                QVERIFY2(checkId(data.ids[0]), "texture load id is not valid");
                QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing texture load url");
            }
            prevTextureSize = data.subdata1;
            break;
        case QQuick3DProfiler::Quick3DGenerateShader:
            QVERIFY2(checkId(data.ids[0]), "generate shader id is not valid");
            QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing profiling id data");
            break;
        case QQuick3DProfiler::Quick3DLoadShader:
            QVERIFY2(checkId(data.ids[0]), "load shader id is not valid");
            QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing shader load url");
            break;
        case QQuick3DProfiler::Quick3DParticleUpdate:
            QVERIFY2(checkId(data.ids[0]), "particle update id is not valid");
            QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing profiling id data");
            break;
        case QQuick3DProfiler::Quick3DRenderCall:
            QVERIFY2(data.subdata1 != 0, "render call is missing payload");
            QVERIFY2(checkId(data.ids[0]), "render call id is not valid");
            QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing render call description");
            if (data.ids[1] != 0) {
                QVERIFY2(checkId(data.ids[0]), "render call id is not valid");
                QVERIFY2(adapter.m_eventData.contains(data.ids[1]&ID_MASK), "Missing profiling id data");
            }
            break;
        case QQuick3DProfiler::Quick3DRenderPass:
            QVERIFY2(checkId(data.ids[0]), "render pass id is not valid");
            QVERIFY2(adapter.m_eventData.contains(data.ids[0]&ID_MASK), "Missing render pass description");
            break;
        case QQuick3DProfiler::Quick3DEventData:
            QVERIFY2(adapter.m_eventData.contains(data.subdata1), "Profiling event data not found");
            break;
        default:
            QFAIL("Invalid detail type");
            break;
        }
    }
    for (int i = 0; i < QQuick3DProfiler::MaximumQuick3DFrameType; i++) {
        QString msg("Required message detail type not received: %1");
        msg = msg.arg(i);
        QVERIFY2(messageReceived[i] == true, qPrintable(msg));
    }
}

QTEST_MAIN(tst_QQuick3DProfiler)
#include "tst_qquick3dprofiler.moc"

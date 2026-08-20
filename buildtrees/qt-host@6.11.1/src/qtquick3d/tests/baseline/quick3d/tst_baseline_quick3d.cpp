// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qbaselinetest.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDirIterator>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtGui/QImage>

#include <algorithm>

// qmlscenegrabber's default timeout, in ms
#define SCENE_TIMEOUT 6000

static bool hasXrEnvironment()
{
    static bool val = qEnvironmentVariableIntValue("LANCELOT_XR_ENVIRONMENT") != 0;
    return val;
}

static quint16 checksumFileOrDir(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isReadable())
        return 0;
    if (fi.isFile()) {
        QFile f(path);
        bool isBinary = path.endsWith(".png") || path.endsWith(".jpg");
        if (!f.open(isBinary ? QIODevice::ReadOnly : QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical() << "Failed to open file" << path << f.errorString();
            return 0;
        }

        QByteArray contents = f.readAll();
        return qChecksum(contents);
    }
    if (fi.isDir()) {
        static const QStringList nameFilters = QStringList() << "*.qml" << "*.cpp" << "*.png" << "*.jpg";
        quint16 cs = 0;
        const auto entryList = QDir(fi.filePath()).entryList(nameFilters,
                                                             QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &item : entryList)
            cs ^= checksumFileOrDir(path + QLatin1Char('/') + item);
        return cs;
    }
    return 0;
}

class tst_Quick3D : public QObject
{
    Q_OBJECT

public:
    tst_Quick3D();

private Q_SLOTS:
    void initTestCase();
    void cleanup();
    void cleanupTestCase();
    void testRendering_data();
    void testRendering();
    void xrTestRendering_data();
    void xrTestRendering();

private:
    void runTest(const QString &qmlFile, const QString &grabber, const QStringList& extraArgs = QStringList());
    bool renderAndGrab(const QString& qmlFile, const QString &grabber, const QStringList& extraArgs, QImage *screenshot, QString *errMsg);
    quint16 checksumFileOrDir(const QString &path);

    QString testSuitePath;
    QString testSuitePathXr;
    int grabberTimeout;
    int consecutiveErrors;   // Not test failures (image mismatches), but system failures (so no image at all)
    bool aborted;            // This run given up because of too many system failures
};

enum class TestData : size_t {
    QmlFile,
    QmlFileXr,
};

static const char testDataTags[][10] = {
    "qmlFile",
    "qmlFileXr",
};

tst_Quick3D::tst_Quick3D()
    : consecutiveErrors(0), aborted(false)
{
    int sceneTimeout = qEnvironmentVariableIntValue("LANCELOT_SCENE_TIMEOUT");
    if (!sceneTimeout)
        sceneTimeout = SCENE_TIMEOUT;
    grabberTimeout = (sceneTimeout * 4) / 3; // Include some slack
}

void tst_Quick3D::initTestCase()
{
    {
        QString dataDir = QFINDTESTDATA("../data/.");
        if (dataDir.isEmpty())
            dataDir = QStringLiteral("data");
        QFileInfo fi(dataDir);
        if (!fi.exists() || !fi.isDir() || !fi.isReadable())
            QSKIP("Test suite data directory missing or unreadable: " + fi.canonicalFilePath().toLatin1());
        testSuitePath = fi.canonicalFilePath();
    }

    if (hasXrEnvironment()) {
        QString dataDirXr = QFINDTESTDATA("../data_xr/.");
        if (dataDirXr.isEmpty())
            dataDirXr = QStringLiteral("data_xr");
        QFileInfo fiXr(dataDirXr);
        if (!fiXr.exists() || !fiXr.isDir() || !fiXr.isReadable())
            QSKIP("XR test suite data directory missing or unreadable: " + fiXr.canonicalFilePath().toLatin1());
        testSuitePathXr = fiXr.canonicalFilePath();
    }

#if defined(Q_OS_WIN)
    const char *defaultRhiBackend = "d3d11";
#elif defined(Q_OS_DARWIN)
    const char *defaultRhiBackend = "metal";
#else
    const char *defaultRhiBackend = "opengl";
#endif
    const QString rhiBackend = qEnvironmentVariable("QSG_RHI_BACKEND", QString::fromLatin1(defaultRhiBackend));
    const QString stack = QString::fromLatin1("RHI_%1").arg(rhiBackend);
    QBaselineTest::addClientProperty(QString::fromLatin1("GraphicsStack"), stack);

    QByteArray msg;
    if (!QBaselineTest::connectToBaselineServer(&msg))
        QSKIP(msg);
}


void tst_Quick3D::cleanup()
{
    // Allow subsystems time to settle
    if (!aborted)
        QTest::qWait(grabberTimeout / 100);
}

void tst_Quick3D::cleanupTestCase()
{
    QBaselineTest::finalizeAndDisconnect();
}

template <TestData Selector>
void setupTestSuite(const QString &testSuitePath, const QByteArray& filter = {})
{
    const char *columnTag = testDataTags[static_cast<size_t>(Selector)];
    QTest::addColumn<QString>(columnTag);
    int numItems = 0;

    QStringList ignoreItems;
    QFile ignoreFile(testSuitePath + "/Ignore");
    if (ignoreFile.open(QIODevice::ReadOnly)) {
        while (!ignoreFile.atEnd()) {
            QByteArray line = ignoreFile.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith('#'))
                ignoreItems += line;
        }
    }

    QStringList itemFiles;
    QDirIterator it(testSuitePath, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString fp = it.next();
        if (fp.endsWith(".qml")) {
            QString itemName = fp.mid(testSuitePath.length() + 1);
            if (!ignoreItems.contains(itemName) && (filter.isEmpty() || !itemName.startsWith(filter)))
                itemFiles.append(it.filePath());
        }
    }

    std::sort(itemFiles.begin(), itemFiles.end());
    for (const QString &filePath : std::as_const(itemFiles)) {
        QByteArray itemName = filePath.mid(testSuitePath.length() + 1).toLatin1();
        QBaselineTest::newRow(itemName, checksumFileOrDir(filePath)) << filePath;
        numItems++;
    }

    if (!numItems)
        QSKIP("No .qml test files found in " + testSuitePath.toLatin1());
}

void tst_Quick3D::testRendering_data()
{
    setupTestSuite<TestData::QmlFile>(testSuitePath);
    consecutiveErrors = 0;
    aborted = false;
}


void tst_Quick3D::testRendering()
{
    QString grabber("qquick3d_qmlscenegrabber");
    QFETCH(QString, qmlFile);
    runTest(qmlFile, grabber);
}

void tst_Quick3D::xrTestRendering_data()
{
    if (!hasXrEnvironment())
        QSKIP("XR test environment not set up for this test run.");

    setupTestSuite<TestData::QmlFileXr>(testSuitePathXr);
    consecutiveErrors = 0;
    aborted = false;
}

void tst_Quick3D::xrTestRendering()
{
    QString grabber("qquick3d_qmlscenegrabber_xr");
    QFETCH(QString, qmlFileXr);
    // NOTE: Set scale to avoid creating large images. Unlike the regular test,
    //       we're more interested in the XR rendering itself than the image quality or
    //       fine details.
    // NOTE: Aspect ratio is preserved, so this migth differ slightly from the
    //       actual resolution of the output.
    runTest(qmlFileXr, grabber, {"-s", "800x600"});
}

void tst_Quick3D::runTest(const QString &qmlFile, const QString &grabber, const QStringList& extraArgs)
{
    // qDebug() << "Rendering" << QTest::currentDataTag();

    if (aborted)
        QSKIP("System too unstable.");

    QImage screenShot;
    QString errorMessage;

    if (renderAndGrab(qmlFile, grabber, extraArgs, &screenShot, &errorMessage)) {
        consecutiveErrors = 0;
    } else {
        if (++consecutiveErrors >= 3 && QBaselineTest::shouldAbortIfUnstable())
            aborted = true;                   // Just give up if screen grabbing fails 3 times in a row
        QFAIL(qPrintable("QuickView grabbing failed: " + errorMessage));
    }

    QBASELINE_TEST(screenShot);
}


bool tst_Quick3D::renderAndGrab(const QString& qmlFile, const QString &grabberExec, const QStringList& extraArgs, QImage *screenshot, QString *errMsg)
{
    bool usePipe = true;  // Whether to transport the grabbed image using temp. file or pipe. TBD: cmdline option
#if defined(Q_OS_WIN)
    usePipe = false;
#endif
    QProcess grabber;
    grabber.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QString cmd = QCoreApplication::applicationDirPath() + QDir::separator() + grabberExec;
    QStringList args = extraArgs;
#if defined(Q_OS_WIN)
    args << "-platform" << "windows:fontengine=freetype";
#elif defined(Q_OS_DARWIN)
    args << "-platform" << "cocoa:fontengine=freetype";
#endif

    QString tmpfile = usePipe ? QString("-") : QString("%1/qmlscenegrabber-%2-out.ppm")
                                .arg(QDir::tempPath()).arg(QCoreApplication::applicationPid());
    args << qmlFile << "-o" << tmpfile;
    grabber.start(cmd, args, QIODevice::ReadOnly);
    grabber.waitForFinished(grabberTimeout);
    if (grabber.state() != QProcess::NotRunning) {
        grabber.terminate();
        grabber.waitForFinished(grabberTimeout / 4);
    }
    QImage img;
    bool res = usePipe ? img.load(&grabber, "ppm") : img.load(tmpfile);
    if (!res || img.isNull()) {
        if (errMsg) {
            QString s("Failed to grab screen. qmlscenegrabber exitcode: %1. Process error: %2.");
            *errMsg = s.arg(grabber.exitCode()).arg(grabber.errorString());
        }
        if (!usePipe)
            QFile::remove(tmpfile);
        return false;
    }
    if (screenshot)
        *screenshot = img;
    if (!usePipe)
        QFile::remove(tmpfile);
    return true;
}

QBASELINETEST_MAIN(tst_Quick3D)

#include "tst_baseline_quick3d.moc"

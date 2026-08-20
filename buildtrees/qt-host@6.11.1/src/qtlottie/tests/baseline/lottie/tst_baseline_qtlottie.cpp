// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qbaselinetest.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDirIterator>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtGui/QImage>

#include <algorithm>

// qmlscenegrabber's default timeout, in ms
#define SCENE_TIMEOUT 8000

class tst_QtLottie : public QObject
{
    Q_OBJECT

public:
    tst_QtLottie();

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();
    void testRendering_data();
    void testRendering();

private:
    void setupTestSuite(const QByteArray& filter = QByteArray());
    void runTest(const QStringList& extraArgs = QStringList());
    bool renderAndGrab(const QString& jsonFile, const QStringList& extraArgs, QImage *screenshot, QString *errMsg);
    quint16 checksumFileOrDir(const QString &path);

    QString testSuitePath;
    QString grabberPath;
    int grabberTimeout;
    int consecutiveErrors;   // Not test failures (image mismatches), but system failures (so no image at all)
    bool aborted;            // This run given up because of too many system failures
};


tst_QtLottie::tst_QtLottie()
    : consecutiveErrors(0), aborted(false)
{
    int sceneTimeout = qEnvironmentVariableIntValue("LANCELOT_SCENE_TIMEOUT");
    if (!sceneTimeout)
        sceneTimeout = SCENE_TIMEOUT;
    grabberTimeout = (sceneTimeout * 4) / 3; // Include some slack
}


void tst_QtLottie::initTestCase()
{
    QString dataDir = QFINDTESTDATA("../data/.");
    if (dataDir.isEmpty())
        dataDir = QStringLiteral("data");
    QFileInfo fi(dataDir);
    if (!fi.exists() || !fi.isDir() || !fi.isReadable())
        QSKIP("Test suite data directory missing or unreadable: " + fi.canonicalFilePath().toLatin1());
    testSuitePath = fi.canonicalFilePath();

#if defined(Q_OS_WIN)
    grabberPath = QFINDTESTDATA("qtlottie_qmlscenegrabber.exe");
#else
    grabberPath = QFINDTESTDATA("qtlottie_qmlscenegrabber");
#endif
    if (grabberPath.isEmpty())
        grabberPath = QCoreApplication::applicationDirPath() + "/qtlottie_qmlscenegrabber";

    const char *backendVarName = "QT_QUICK_BACKEND";
    const QString backend = qEnvironmentVariable(backendVarName, QString::fromLatin1("default"));
    QBaselineTest::addClientProperty(QString::fromLatin1(backendVarName), backend);

    QString stack = backend;
    if (stack != QLatin1String("software")) {
#if defined(Q_OS_WIN)
        const char *defaultRhiBackend = "d3d11";
#elif defined(Q_OS_DARWIN)
        const char *defaultRhiBackend = "metal";
#else
        const char *defaultRhiBackend = "opengl";
#endif
        const QString rhiBackend = qEnvironmentVariable("QSG_RHI_BACKEND", QString::fromLatin1(defaultRhiBackend));
        stack = QString::fromLatin1("RHI_%1").arg(rhiBackend);
    }
    QBaselineTest::addClientProperty(QString::fromLatin1("GraphicsStack"), stack);

    QByteArray msg;
    if (!QBaselineTest::connectToBaselineServer(&msg))
        QSKIP(msg);
}

void tst_QtLottie::init()
{
    // This gets called for every row. QSKIP if current item is blacklisted on the baseline server:
    QBASELINE_SKIP_IF_BLACKLISTED;
}

void tst_QtLottie::cleanup()
{
    // Allow subsystems time to settle
    if (!aborted)
        QTest::qWait(grabberTimeout / 100);
}

void tst_QtLottie::cleanupTestCase()
{
    QBaselineTest::finalizeAndDisconnect();
}

void tst_QtLottie::testRendering_data()
{
    setupTestSuite();
    consecutiveErrors = 0;
    aborted = false;
}


void tst_QtLottie::testRendering()
{
    runTest();
}


void tst_QtLottie::setupTestSuite(const QByteArray& filter)
{
    QTest::addColumn<QString>("jsonFile");
    QTest::addColumn<int>("frameNumber");
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
        if (fp.endsWith(".json")) {
            QString itemName = fp.mid(testSuitePath.size() + 1);
            if (!ignoreItems.contains(itemName) && (filter.isEmpty() || !itemName.startsWith(filter)))
                itemFiles.append(it.filePath());
        }
    }

    std::sort(itemFiles.begin(), itemFiles.end());
    for (const QString &filePath : std::as_const(itemFiles)) {
        QByteArray itemName = filePath.mid(testSuitePath.size() + 1).toLatin1();
        quint16 checksum = checksumFileOrDir(filePath);
        QBaselineTest::newRow(itemName, checksum) << filePath << -1;
        numItems++;

#if 0
        // Example of simple way of adding test for non-default frame
        if (filePath.contains("freeze/")) {
            int frameNo = 71;
            QByteArray frameItemName = itemName + "_" + QByteArray::number(frameNo);
            QBaselineTest::newRow(frameItemName, checksum) << filePath << frameNo;
            numItems++;
        }
#endif
    }

    if (!numItems)
        QSKIP("No .json test files found in " + testSuitePath.toLatin1());
}


void tst_QtLottie::runTest(const QStringList& extraArgs)
{
    if (aborted)
        QSKIP("System too unstable.");

    QFETCH(QString, jsonFile);
    QFETCH(int, frameNumber);

    QStringList args = extraArgs;
    if (frameNumber >= 0)
        args << "-frame" << QString::number(frameNumber);

    QImage screenShot;
    QString errorMessage;
    if (renderAndGrab(jsonFile, args, &screenShot, &errorMessage)) {
        consecutiveErrors = 0;
    } else {
        if (++consecutiveErrors >= 3 && QBaselineTest::shouldAbortIfUnstable())
            aborted = true; // Just give up if screen grabbing fails 3 times in a row
        QFAIL(qPrintable("QuickView grabbing failed: " + errorMessage));
    }

    QBASELINE_TEST(screenShot);
}


bool tst_QtLottie::renderAndGrab(const QString& jsonFile, const QStringList& extraArgs, QImage *screenshot, QString *errMsg)
{
    bool usePipe = true;  // Whether to transport the grabbed image using temp. file or pipe.
    QProcess grabber;
    grabber.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QStringList args = extraArgs;
    QString tmpfile = usePipe ? QString("-") : QString("/tmp/qmlscenegrabber-%1-out.ppm").arg(QCoreApplication::applicationPid());
    args << jsonFile << "-o" << tmpfile;
    grabber.start(grabberPath, args, QIODevice::ReadOnly);
    grabber.waitForFinished(grabberTimeout);
    if (grabber.state() != QProcess::NotRunning) {
        grabber.terminate();
        grabber.waitForFinished(grabberTimeout / 4);
    }
    QImage img;
    bool res = usePipe ? img.load(&grabber, "ppm") : img.load(tmpfile);
    if (!res || img.isNull()) {
        if (errMsg) {
            QString s("Failed to grab screen. qmlscenegrabber exitcode: %1. Process error: %2. Stderr:\n%3");
            *errMsg = s.arg(grabber.exitCode()).arg(grabber.errorString()).arg(grabber.readAllStandardError());
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


quint16 tst_QtLottie::checksumFileOrDir(const QString &path)
{
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isReadable())
        return 0;
    if (fi.isFile()) {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly))
            qFatal("Could not open file %s", qPrintable(path));
        QByteArray contents = f.readAll();
        return qChecksum(contents);
    }
    if (fi.isDir()) {
        static const QStringList nameFilters = QStringList() << "*.qml" << "*.json" << "*.png";
        quint16 cs = 0;
        const auto entryList = QDir(fi.filePath()).entryList(nameFilters,
                                                             QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        for (const QString &item : entryList)
            cs ^= checksumFileOrDir(path + QLatin1Char('/') + item);
        return cs;
    }
    return 0;
}

QBASELINETEST_MAIN(tst_QtLottie)

#include "tst_baseline_qtlottie.moc"

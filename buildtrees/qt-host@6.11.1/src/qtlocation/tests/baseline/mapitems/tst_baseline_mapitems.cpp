// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qbaselinetest.h>

#include <QtCore/QDirIterator>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QPalette>
#include <QtGui/QFont>

#include <algorithm>

QString blockify(const QByteArray& s)
{
    const char* indent = "\n | ";
    QByteArray block = s.trimmed();
    block.replace('\n', indent);
    block.prepend(indent);
    block.append('\n');
    return block;
}

class tst_Baseline_MapItems : public QObject
{
    Q_OBJECT

public:
    tst_Baseline_MapItems();

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void mapItems_data() { setupTestSuite(); }
    void mapItems() { runTest(); }

private:
    void test();

    void setupTestSuite();
    void runTest();
    bool renderAndGrab(const QString& qmlFile, const QStringList& extraArgs, QImage *screenshot, QString *errMsg);

    QString testSuitePath;
    QString grabberPath;
    QStringList testFiles;
    int consecutiveErrors;   // Not test failures (image mismatches), but system failures (so no image at all)
    bool aborted;            // This run given up because of too many system failures
};


tst_Baseline_MapItems::tst_Baseline_MapItems()
    : consecutiveErrors(0), aborted(false)
{
}


void tst_Baseline_MapItems::initTestCase()
{
    QString dataDir = QFINDTESTDATA("./data/.");
    if (dataDir.isEmpty())
        dataDir = QStringLiteral("data");
    QFileInfo fi(dataDir);
    if (!fi.exists() || !fi.isDir() || !fi.isReadable())
        QSKIP("Test suite data directory missing or unreadable: " + fi.canonicalFilePath().toLatin1());
    testSuitePath = fi.canonicalFilePath();

#if defined(Q_OS_WIN)
    grabberPath = QFINDTESTDATA("../../../../qtdeclarative/tests/baseline/scenegraph/qmlscenegrabber.exe");
#elif defined(Q_OS_DARWIN)
    grabberPath = QFINDTESTDATA("../../../../qtdeclarative/tests/baseline/scenegraph/qmlscenegrabber.app/Contents/MacOS/qmlscenegrabber");
#else
    grabberPath = QFINDTESTDATA("../../../../qtdeclarative/tests/baseline/scenegraph/qmlscenegrabber");
#endif
    if (grabberPath.isEmpty())
        grabberPath = QCoreApplication::applicationDirPath() + "../../../../qtdeclarative/tests/baseline/scenegraph/qmlscenegrabber.exe";

    QBaselineTest::setProject("MapItems");
    // Set key platform properties that are relevant for the appearance of controls
    const QString platformName = QGuiApplication::platformName() + "-" + QSysInfo::productType();
    QBaselineTest::addClientProperty("PlatformName", platformName);
    QBaselineTest::addClientProperty("OSVersion", QSysInfo::productVersion());

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

    // Assume that text that's darker than the background means we run in light mode
    // See also qwidgetbaselinetest.cpp
    QPalette palette;
    QFont font;
    QByteArray appearanceBytes;
    {
        QDataStream appearanceStream(&appearanceBytes, QIODevice::WriteOnly);
        appearanceStream << palette << font;
        const qreal screenDpr = QGuiApplication::primaryScreen()->devicePixelRatio();
        if (screenDpr != 1.0) {
            qWarning() << "DPR is" << screenDpr << "- images will not be compared to 1.0 baseline!";
            appearanceStream << screenDpr;
        }
    }
    const quint16 appearanceId = qChecksum(appearanceBytes);

    const QColor windowColor = palette.window().color();
    const QColor textColor = palette.text().color();
    const QString appearanceIdString = (windowColor.value() > textColor.value()
                                        ? QString("light-%1") : QString("dark-%1"))
                                        .arg(appearanceId, 0, 16);
    QBaselineTest::addClientProperty("AppearanceID", appearanceIdString);

    QByteArray msg;
    if (!QBaselineTest::connectToBaselineServer(&msg))
        QSKIP(msg);
}

void tst_Baseline_MapItems::init()
{
    consecutiveErrors = 0;
    aborted = false;
}

void tst_Baseline_MapItems::cleanup()
{
    // Allow subsystems time to settle
    if (!aborted)
        QTest::qWait(20);
}

void tst_Baseline_MapItems::cleanupTestCase()
{
    QBaselineTest::finalizeAndDisconnect();
}

void tst_Baseline_MapItems::setupTestSuite()
{
    QTest::addColumn<QString>("qmlFile");

    const QStringView qmlExt(u".qml");
    if (testFiles.isEmpty()) {
        QDirIterator it(testSuitePath, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString fp = it.next().toLatin1();
            if (fp.endsWith(qmlExt))
                testFiles.append(it.filePath());
        }
        std::sort(testFiles.begin(), testFiles.end());
    }

    if (testFiles.isEmpty())
        QSKIP("No .qml test files found in " + testSuitePath.toLatin1());

    for (const auto &filePath : std::as_const(testFiles)) {
        QString itemName = filePath.sliced(testSuitePath.size() + 1);
        itemName = itemName.left(itemName.size() - qmlExt.size());
        QBaselineTest::newRow(itemName.toLatin1()) << filePath;
    }
}


void tst_Baseline_MapItems::runTest()
{
    if (aborted)
        QSKIP("System too unstable.");

    QFETCH(QString, qmlFile);

    QImage screenShot;
    QString errorMessage;
    QStringList args;
    if (renderAndGrab(qmlFile, args, &screenShot, &errorMessage)) {
        consecutiveErrors = 0;
    } else {
        if (++consecutiveErrors >= 3)
            aborted = true;                   // Just give up if screen grabbing fails 3 times in a row
        QFAIL(qPrintable("QuickView grabbing failed: " + errorMessage));
    }

    QBASELINE_TEST(screenShot);
}


bool tst_Baseline_MapItems::renderAndGrab(const QString& qmlFile, const QStringList& extraArgs, QImage *screenshot, QString *errMsg)
{
    bool usePipe = true;  // Whether to transport the grabbed image using temp. file or pipe. TBD: cmdline option
    QProcess grabber;
    grabber.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    QStringList args = extraArgs;
    QString tmpfile = usePipe ? QString("-") : QString("/tmp/qmlscenegrabber-%1-out.ppm").arg(QCoreApplication::applicationPid());
    args << qmlFile << "-o" << tmpfile;
    grabber.start(grabberPath, args, QIODevice::ReadOnly);
    grabber.waitForFinished(17000);         //### hardcoded, must be larger than the scene timeout in qmlscenegrabber
    if (grabber.state() != QProcess::NotRunning) {
        grabber.terminate();
        grabber.waitForFinished(3000);
    }
    QImage img;
    bool res = usePipe ? img.load(&grabber, "ppm") : img.load(tmpfile);
    if (!res || img.isNull()) {
        if (errMsg) {
            QString s("Failed to grab screen. qmlscenegrabber exitcode: %1. Process error: %2. Stderr:%3");
            *errMsg = s.arg(grabber.exitCode()).arg(grabber.errorString()).arg(blockify(grabber.readAllStandardError()));
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

QBASELINETEST_MAIN(tst_Baseline_MapItems)

#include "tst_baseline_mapitems.moc"

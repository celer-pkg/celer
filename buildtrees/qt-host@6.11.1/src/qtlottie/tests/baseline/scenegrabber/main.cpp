// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QHashFunctions>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtCore/QLoggingCategory>
#include <QtCore/QScopedValueRollback>

#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>
#include <QQmlApplicationEngine>
#include <QtQml/qqmlcomponent.h>

#ifdef Q_OS_WIN
#  include <fcntl.h>
#  include <io.h>
#endif // Q_OS_WIN

// Timeout values:

// A valid screen grab requires the scene to not change
// for SCENE_STABLE_TIME ms
#define SCENE_STABLE_TIME 200

// Give up after SCENE_TIMEOUT ms
#define SCENE_TIMEOUT     8000

Q_LOGGING_CATEGORY(lcGrabber, "qt.baseline.scenegrabber")

static const QSize DefaultGrabSize(800, 800);

class GrabbingView : public QQuickView
{
    Q_OBJECT

public:
    GrabbingView(const QString &outputFile)
        : ofile(outputFile), justShow(outputFile.isEmpty())
    {
        if (justShow)
            return;
        grabTimer = new QTimer(this);
        grabTimer->setSingleShot(true);
        grabTimer->setInterval(SCENE_STABLE_TIME);
        connect(grabTimer, &QTimer::timeout, this, &GrabbingView::grab);

        QObject::connect(this, &QQuickWindow::afterRendering, this, &GrabbingView::startGrabbing);

        int sceneTimeout = qEnvironmentVariableIntValue("LANCELOT_SCENE_TIMEOUT");
        if (!sceneTimeout)
            sceneTimeout = SCENE_TIMEOUT;
        QTimer::singleShot(sceneTimeout, this, &GrabbingView::timedOut);
    }

private slots:
    void startGrabbing()
    {
        qCDebug(lcGrabber) << "Restarting grab timer";
        grabTimer->start();
    }

    void grab()
    {
        if (isGrabbing) {
            qCDebug(lcGrabber) << "Already grabbing, skipping";
            return;
        }
        QScopedValueRollback grabGuard(isGrabbing, true);

        grabNo++;
        qCDebug(lcGrabber) << "Starting grab no." << grabNo;
        QImage img = grabWindow();
        qCDebug(lcGrabber) << "Finishing grab no." << grabNo;
        if (!img.isNull() && img == lastGrab) {
            sceneStabilized();
        } else {
            lastGrab = img;
            grabTimer->start();
        }
    }

    void sceneStabilized()
    {
        qCDebug(lcGrabber) << "...sceneStabilized IN";
        if (QGuiApplication::platformName() == QLatin1String("eglfs")) {
            QSize rootSize;
            if (QQuickItem *r = rootObject())
                rootSize = (QSizeF(r->width(), r->height()) * effectiveDevicePixelRatio()).toSize();
            QSize grabSize = rootSize.isEmpty() ? DefaultGrabSize : rootSize;
            lastGrab = lastGrab.copy(QRect(QPoint(0, 0), grabSize));
        }

        if (ofile == "-") {   // Write to stdout
            QFile of;
#ifdef Q_OS_WIN
            // Make sure write to stdout doesn't do LF->CRLF
            _setmode(_fileno(stdout), _O_BINARY);
#endif // Q_OS_WIN
            if (!of.open(1, QIODevice::WriteOnly) || !lastGrab.save(&of, "ppm")) {
                qWarning() << "Error: failed to write grabbed image to stdout.";
                QGuiApplication::exit(2);
                return;
            }
        } else {
            if (!lastGrab.save(ofile)) {
                qWarning() << "Error: failed to store grabbed image to" << ofile;
                QGuiApplication::exit(2);
                return;
            }
        }
        QGuiApplication::exit(0);
        qCDebug(lcGrabber) << "...sceneStabilized OUT";
    }

    void timedOut()
    {
        qWarning() << "Error: timed out waiting for scene to stabilize." << grabNo << "grab(s) done. Last grab was" << (lastGrab.isNull() ? "invalid." : "valid.");
        QGuiApplication::exit(3);
    }

private:
    QImage lastGrab;
    QTimer *grabTimer = nullptr;
    QString ofile;
    int grabNo = 0;
    bool isGrabbing = false;
    bool initDone = false;
    bool justShow = false;
};


int main(int argc, char *argv[])
{
    QHashSeed::setDeterministicGlobalSeed();

    QGuiApplication a(argc, argv);

    // Parse command line
    QString ifile, ofile, style;
    bool justShow = false;
    int freezeFrame = -1;

    QStringList args = a.arguments();
    int i = 0;
    bool argError = false;
    while (++i < args.size()) {
        QString arg = args.at(i);
        if ((arg == "-o") && (i < args.size()-1)) {
            ofile = args.at(++i);
        }
        else if ((arg == "-frame") && (i < args.size() - 1)) {
            bool ok = false;
            freezeFrame = args.at(++i).toInt(&ok);
            if (!ok || freezeFrame < 0) {
                argError = true;
                break;
            }
        }
        else if (arg == "-viewonly") {
            justShow = true;
        }
        else if (ifile.isEmpty()) {
            ifile = arg;
        }
        else {
            argError = true;
            break;
        }
    }
    if (argError || ifile.isEmpty() || (ofile.isEmpty() && !justShow)) {
        qWarning() << "Usage:" << args.at(0).toLatin1().constData()
                   << "[-frame framenumber] <Lottie JSON infile> {-o <outfile or - for ppm on stdout>|-viewonly}";
        return 1;
    }

    QFileInfo ifi(ifile);
    if (!ifi.exists() || !ifi.isReadable() || !ifi.isFile()) {
        qWarning() << args.at(0).toLatin1().constData() << " error: unreadable input file" << ifile;
        return 1;
    }
    // End parsing

    qputenv("QLT_FRAMENO", QByteArray::number(freezeFrame));

    GrabbingView v(ofile);
    v.loadFromModule("QtLottieAnimation", "Main");
    QQuickItem *lottieItem = v.rootObject()->findChild<QQuickItem *>("qtlottie_animation_item");
    Q_ASSERT(lottieItem);
    if (freezeFrame >= 0)
        lottieItem->setProperty("freezeFrame", freezeFrame);
    lottieItem->setProperty("source", QUrl::fromLocalFile(ifile));

    if (v.initialSize().isEmpty())
        v.resize(DefaultGrabSize);

    v.show();

    int retVal = a.exec();
    qCDebug(lcGrabber) << "...retVal=" << retVal;

    return retVal;
}

#include "main.moc"

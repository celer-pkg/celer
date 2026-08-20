// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mainwindow.h"

#include <QApplication>
#include <QScreen>
#include <QAxFactory>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

QAXFACTORY_BEGIN(
    "{4a43e44d-9d1d-47e5-a1e5-58fe6f7be0a4}", // type library ID
    "{16ee5998-77d2-412f-ad91-8596e29f123f}") // application ID
    QAXCLASS(MainWindow)
QAXFACTORY_END()

QT_USE_NAMESPACE

static bool isOptionSet(int argc, char *argv[], const char *option)
{
    for (int i = 1; i < argc; ++i) {
        if (!qstrcmp(argv[i], option))
            return true;
    }
    return false;
}

static void redirectDebugOutput(QtMsgType, const QMessageLogContext &, const QString &msg)
{
    if (MainWindow *mainWindow = MainWindow::instance())
        mainWindow->appendLogText(msg);
}

int main( int argc, char **argv )
{
    if (isOptionSet(argc, argv, "--no-native-siblings"))
        QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app( argc, argv );
    QCoreApplication::setApplicationName(QLatin1String("TestCon"));
    QCoreApplication::setOrganizationName(QLatin1String("QtProject"));
    QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));
    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String("ActiveX Control Test Container"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption scriptOption(QLatin1String("script"),
                                    QLatin1String("A script to load."),
                                    QLatin1String("script"));
    parser.addOption(scriptOption);
    QCommandLineOption noMessageHandlerOption(QLatin1String("no-messagehandler"),
                                              QLatin1String("Suppress installation of the message handler."));
    parser.addOption(noMessageHandlerOption);
    QCommandLineOption noNativeSiblingsDummy(QLatin1String("no-native-siblings"),
                                             QLatin1String("Set Qt::AA_DontCreateNativeWidgetSiblings."));
    parser.addOption(noNativeSiblingsDummy);
    parser.addPositionalArgument(QLatin1String("clsid/file"),
                                 QLatin1String("The clsid/file to show."));
    parser.process(app);

    if (!parser.isSet(noMessageHandlerOption))
        qInstallMessageHandler(redirectDebugOutput);

    MainWindow mw;
    const QStringList positionalArguments = parser.positionalArguments();
    for (const QString &a : positionalArguments) {
        if (a.startsWith(QLatin1Char('{')) && a.endsWith(QLatin1Char('}')))
            mw.addControlFromClsid(a, QAxSelect::SandboxingNone);
        else
            mw.addControlFromFile(a);
    }
    if (parser.isSet(scriptOption))
        mw.loadScript(parser.value(scriptOption));

    const QRect availableGeometry = mw.screen()->availableGeometry();
    mw.resize(availableGeometry.size() * 2 / 3);
    mw.show();

    return app.exec();;
}

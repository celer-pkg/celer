// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QtQuick3D>

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcommandlineoption.h>

// For component loading case
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK3D_EDITORMODE", "1");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    app.setApplicationName("Material Editor");
    app.setOrganizationName("QtProject");
    app.setOrganizationDomain("qt-project.org");
    app.setApplicationVersion(QLatin1String(QT_VERSION_STR));

    QSurfaceFormat::setDefaultFormat(QQuick3D::idealSurfaceFormat(4));

    QCommandLineParser cmdLineparser;
    cmdLineparser.setApplicationDescription("Editor for creating custom materials for use with Qt Quick 3D");
    cmdLineparser.addHelpOption();

    QCommandLineOption modeOption({QChar(u'm'), QLatin1String("mode")}, QLatin1String("sets editor mode"), QLatin1String("c"));
    cmdLineparser.addOption(modeOption);

    QCommandLineOption changeDirOption({QChar(u'C'), QLatin1String("directory")}, QLatin1String("Change the working directory"), QLatin1String("dir"));
    cmdLineparser.addOption(changeDirOption);

    QCommandLineOption projectDirOption({QChar(u'p'), QLatin1String("project-dir")}, QLatin1String("Project directory"), QLatin1String("dir"));
    cmdLineparser.addOption(projectDirOption);

    QCommandLineOption verboseOutputOption({QChar(u'v'), QLatin1String("verbose")}, QLatin1String("Turn on verbose output."));
    cmdLineparser.addOption(verboseOutputOption);

    cmdLineparser.process(app);

    bool workingDirSet = false;
    if (cmdLineparser.isSet(changeDirOption)) {
        const auto value = cmdLineparser.value(changeDirOption);
        QFileInfo fi(value);
        workingDirSet = fi.isDir();
        if (workingDirSet)
            QDir::setCurrent(value);
        else
            qWarning("%s : %s - Not a directory", qPrintable(app.applicationName()), qPrintable(value));
    }

    QString resourceFile;
    auto args = cmdLineparser.positionalArguments();
    for (const auto &arg : std::as_const(args)) {
        if (arg.endsWith(".qml"))
            resourceFile = arg;
    }

    if (!resourceFile.isEmpty() && QFileInfo::exists(resourceFile)) {
        QFileInfo resourceFileInfo(resourceFile);
        if (resourceFileInfo.isFile()) {
            if (!workingDirSet)
                QDir::setCurrent(resourceFileInfo.dir().path());

            resourceFile = resourceFileInfo.canonicalFilePath();
        }
    }

    QString projectPath;
    if (cmdLineparser.isSet(projectDirOption)) {
        const auto value = cmdLineparser.value(projectDirOption);
        QFileInfo fi(value);
        if (fi.isDir())
            projectPath = fi.canonicalFilePath();
        else
            qWarning("%s : %s - Not a directory", qPrintable(app.applicationName()), qPrintable(value));
    }

    QQmlApplicationEngine engine;
    if (auto ctx = engine.rootContext())
        ctx->setContextProperty("_qtProjectDir", QUrl::fromLocalFile(projectPath));
    const QUrl url(QStringLiteral("qrc:/qt-project.org/imports/QtQuick3D/MaterialEditor/main.qml"));
    engine.load(url);
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

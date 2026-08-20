// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "lightmapfile.h"
#include "lightmapimageprovider.h"
#include "lightmapviewerhelpers.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

int main(int argc, char *argv[])
{
    QString input;

    // We Handle all the command line parts first in its own QCoreApplication
    // since we don't want to create a gui application which would not work if
    // running through ssh for instance.
    {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName("Qt LightmapViewer");
        QCoreApplication::setApplicationVersion("1.0");

        QCommandLineParser parser;
        parser.setApplicationDescription("A tool for viewing Qt's baked lightmaps");
        parser.addHelpOption();
        parser.addVersionOption();

        QCommandLineOption printOption("print", "Print the content of the lightmap");
        QCommandLineOption extractOption("extract", "Extract the lightmap contents into the current working directory");
        parser.addOption(printOption);
        parser.addOption(extractOption);

        parser.addPositionalArgument("input", "Optional path to lightmap");

        parser.process(app);

        bool doPrint = parser.isSet(printOption);
        bool doExtract = parser.isSet(extractOption);

        const QStringList positionalArgs = parser.positionalArguments();
        input = positionalArgs.isEmpty() ? QString() : positionalArgs.first();

        if (doPrint && input.isEmpty()) {
            qFatal() << "Print option selected with no lightmap.";
            return 1;
        }

        if (doExtract && input.isEmpty()) {
            qFatal() << "Extract option selected with no lightmap.";
            return 1;
        }

        if (doPrint || doExtract)
            return !LightmapViewerHelpers::processLightmap(input, doPrint, doExtract);
    }

    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    LightmapFile *file = new LightmapFile();
    if (!input.isEmpty())
        file->setSource(QUrl::fromLocalFile(input));
    file->loadData();
    qmlRegisterSingletonInstance("LightmapFile", 1, 0, "LightmapFile", file);
    LightmapImageProvider *provider = new LightmapImageProvider;
    engine.addImageProvider(QLatin1String("lightmaps"), provider);
    engine.loadFromModule("QtQuick3D.lightmapviewer", "LightmapViewer");

    return app.exec();
}

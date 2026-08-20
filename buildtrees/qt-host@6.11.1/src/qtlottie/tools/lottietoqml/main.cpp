// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QQuickWindow>
#include <QQuickItem>
#include <QtQuickVectorImageGenerator/private/qquickitemgenerator_p.h>
#include <QtQuickVectorImageGenerator/private/qquickqmlgenerator_p.h>
#include <QtQuickVectorImageGenerator/private/qquickvectorimageglobal_p.h>
#include <QtLottie/private/qlottieroot_p.h>
#include <QtLottieVectorImageGenerator/private/qlottievisitor_p.h>

#include <QtGui/qpa/qplatformintegrationfactory_p.h>
#include <QtGui/private/qguiapplication_p.h>

int main(int argc, char *argv[])
{

    QStringList arguments;
    for (int i = 0; i < argc; ++i)
        arguments.append(QString::fromLocal8Bit(argv[i]));

    QCommandLineParser parser;
    parser.setApplicationDescription("Lottie to QML converter");
    parser.addHelpOption();
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Lottie file to read."));
    parser.addPositionalArgument("output", QCoreApplication::translate("main", "QML file to write."), "[output]");

    QCommandLineOption curveRendererOption({ "c", "curve-renderer" },
                                           QCoreApplication::translate("main", "Use the curve renderer in generated QML."));
    parser.addOption(curveRendererOption);

    QCommandLineOption optimizeOption({ "p", "optimize-paths" },
                                      QCoreApplication::translate("main", "Optimize paths for the curve renderer."));
    parser.addOption(optimizeOption);

    QCommandLineOption typeNameOption({ "t", "type-name" },
                                      QCoreApplication::translate("main", "Use <typename> for Shape."),
                                      QCoreApplication::translate("main", "typename"));
    parser.addOption(typeNameOption);

    QCommandLineOption copyrightOption("copyright-statement",
                                       QCoreApplication::translate("main", "Add <string> as a comment at the start of the generated file."),
                                       QCoreApplication::translate("main", "string"));
    parser.addOption(copyrightOption);

    QCommandLineOption assetOutputDirectoryOption("asset-output-directory",
                                                  QCoreApplication::translate("main", "If the Lottie file refers to external or embedded files, such as images, these "
                                                                                      "will be copied into the same directory as the output QML file by default. "
                                                                                      "Set the asset output directory to override the location."),
                                                  QCoreApplication::translate("main", "directory"));
    parser.addOption(assetOutputDirectoryOption);

    QCommandLineOption assetOutputPrefixOption("asset-output-prefix",
                                               QCoreApplication::translate("main", "If the Lottie file refers to external or embedded files, such as images, these "
                                                                                   "will be copied to files with unique identifiers. By default, the files will be prefixed "
                                                                                   "with \"lottie_asset_\". Set the asset output prefix to override the prefix."),
                                               QCoreApplication::translate("main", "prefix"));
    assetOutputPrefixOption.setDefaultValue(QStringLiteral("lottie_asset_"));
    parser.addOption(assetOutputPrefixOption);

    QCommandLineOption keepPathsOption("keep-external-paths",
                                       QCoreApplication::translate("main", "Any paths to external files will be retained in the QML output. "
                                                                           "The paths will be reformatted as relative to the output file. If "
                                                                           "this is not enabled, copies of the file will be saved to the asset output "
                                                                           "directory. Embedded data will still be saved to files, even if "
                                                                           "this option is set."));
    parser.addOption(keepPathsOption);

    QCommandLineOption guiOption({ "v", "view" },
                                 QCoreApplication::translate("main", "Display the generated QML in a window. This is the default behavior if no "
                                                                     "output file is specified."));
    parser.addOption(guiOption);
    parser.process(arguments);
    const QStringList args = parser.positionalArguments();
    if (args.size() < 1) {
        parser.showHelp(1);
    }

    const QString inFileName = args.at(0);
    const auto outFileName = args.size() > 1 ? args.at(1) : QString{};

    const bool needsGui = outFileName.isEmpty() || parser.isSet(guiOption);
    const bool useMinimalPlugin = qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")
                                  && !needsGui;
    if (useMinimalPlugin)
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("minimal"));

    QGuiApplication app(argc, argv);

    QString commentString = QStringLiteral("Generated from Lottie file %1").arg(QFileInfo(inFileName).fileName());
    const QString importString = QStringLiteral("Qt.labs.lottieqt.VectorImageHelpers");

    const auto typeName = parser.value(typeNameOption);
    const auto assetOutputDirectory = parser.value(assetOutputDirectoryOption);
    const auto assetOutputPrefix = parser.value(assetOutputPrefixOption);
    const bool keepPaths = parser.isSet(keepPathsOption);
    auto copyrightString = parser.value(copyrightOption);

    if (!copyrightString.isEmpty()) {
        copyrightString = copyrightString.replace("\\n", "\n");
        commentString = copyrightString + u"\n" + commentString;
    }

    QQuickVectorImageGenerator::GeneratorFlags flags;
    if (parser.isSet(curveRendererOption))
        flags |= QQuickVectorImageGenerator::GeneratorFlag::CurveRenderer;
    if (parser.isSet(optimizeOption))
        flags |= QQuickVectorImageGenerator::GeneratorFlag::OptimizePaths;

    QQuickQmlGenerator generator(inFileName, flags, outFileName);
    generator.setShapeTypeName(typeName);
    generator.setCommentString(commentString);
    generator.setAssetFileDirectory(assetOutputDirectory);
    generator.setAssetFilePrefix(assetOutputPrefix);
    generator.setRetainFilePaths(keepPaths);
    generator.addExtraImport(importString);

    int frameNo = qEnvironmentVariableIntValue("QLT_FRAMENO");

    QFile f(inFileName);
    bool ok = false;
    QLottieRoot root;
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray jsonSource = f.readAll();

        if (root.parseSource(jsonSource, QUrl::fromLocalFile(inFileName)) >= 0) {
            root.setStructureDumping(true);
            for (QLottieBase *elem : root.children()) {
                if (elem->active(frameNo))
                    elem->updateProperties(frameNo);
            }

            QLottieVisitor visitor(inFileName, &generator);
            visitor.render(root);
            ok = generator.save();
        }
    }

    if (needsGui && ok) {
        app.setOrganizationName("QtProject");
        const QUrl url(QStringLiteral("qrc:/main.qml"));
        QQmlApplicationEngine engine;
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [&](QObject *obj, const QUrl &objUrl){
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
            if (obj) {
                auto *containerItem = obj->findChild<QQuickItem*>(QStringLiteral("svg_item"));
                QQuickItemGenerator generator(inFileName, flags, containerItem, engine.rootContext());
                generator.addExtraImport(importString);

                QLottieVisitor visitor(inFileName, &generator);
                visitor.render(root);
            }
        });
        engine.load(url);
        return app.exec();
    }

    return ok ? 0 : 1;
}

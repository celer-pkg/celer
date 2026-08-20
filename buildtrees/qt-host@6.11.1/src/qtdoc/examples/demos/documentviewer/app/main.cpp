// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "translator.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>

using namespace Qt::StringLiterals;

struct Tr {
    Q_DECLARE_TR_FUNCTIONS(main);
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("QtProject"_L1);
    QCoreApplication::setApplicationName("DocumentViewer"_L1);
    QCoreApplication::setApplicationVersion("1.0"_L1);

    Translator mainTranslator;
    mainTranslator.setBaseName("docviewer"_L1);
    mainTranslator.install();

    QCommandLineParser parser;
    parser.setApplicationDescription(Tr::tr("A viewer for JSON, PDF and text files"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("File"_L1, Tr::tr("JSON, PDF or text file to open"));
    parser.process(app);

    const QStringList &positionalArguments = parser.positionalArguments();
    const QString &fileName = (positionalArguments.count() > 0) ? positionalArguments.at(0)
                                                                : QString();

    MainWindow w(mainTranslator);

    // Start application only if plugins are available
    if (!w.hasPlugins()) {
        QMessageBox::critical(nullptr,
                              Tr::tr("No viewer plugins found"),
                              Tr::tr("Unable to load viewer plugins. Exiting application."));
        return 1;
    }

    w.show();
    if (!fileName.isEmpty())
        w.openFile(fileName);

    return app.exec();
}

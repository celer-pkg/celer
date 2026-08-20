// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <private/qqmljscontextproperties_p.h>
#include <QtCore/qcommandlineparser.h>

using namespace Qt::StringLiterals;
using namespace QQmlJS;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("qmlcontextpropertydump");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription(R"(QML context property dumper

Runs a heuristic on a folder to find context properties, and dumps them into a .contextProperties.ini for qmllint to read.
)");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption sourceOption(QStringList() << "s"_L1 << "cpp-source-directory"_L1,
                                    "Root source folder where to search for context properties."_L1,
                                    "source"_L1);
    parser.addOption(sourceOption);

    QCommandLineOption buildOption(
            QStringList() << "b"_L1 << "build-directory"_L1,
            "Build folder where to write the .qt/contextPropertyDump.ini file."_L1, "build"_L1);
    parser.addOption(buildOption);

    parser.process(app);

    if (!parser.isSet(sourceOption) || !parser.isSet(buildOption)) {
        parser.showHelp(EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    HeuristicContextProperties contextProperties =
            HeuristicContextProperties::collectFromCppSourceDirs(parser.values(sourceOption));
    contextProperties.writeCache(parser.value(buildOption));
    return EXIT_SUCCESS;
}

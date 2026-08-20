// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <cstdarg>
#include <cstdio>

using namespace Qt::StringLiterals;

static void printError(const char *msg, ...)
{
    va_list arglist;
    va_start(arglist, msg);
    vfprintf(stderr, msg, arglist);
    fputs("\n", stderr);
    va_end(arglist);
}

static QByteArray getSnippet(const QByteArray &name)
{
    const QString fileName = ":/"_L1 + QString::fromUtf8(name);
    QFile f(fileName);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        return f.readAll();
    return {};
}

static bool process(const QString &inputFile, const QString &outputFile)
{
    QFile inFile(inputFile);
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        printError("Failed to open %s", qPrintable(inputFile));
        return false;
    }

    QByteArray content = inFile.readAll();

    QDir().mkpath(QFileInfo(outputFile).path());
    QFile outFile(outputFile);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        printError("Filed to create %s", qPrintable(outputFile));
        return false;
    }

    qsizetype pos = 0;
    for (; ;) {
        const qsizetype foundPos = content.indexOf("QC_INCLUDE ", pos);
        if (foundPos == -1)
            break;

        const qsizetype openQuotePos = content.indexOf('\"', foundPos + 1);
        if (openQuotePos != -1) {
            const qsizetype closeQuotePos = content.indexOf('\"', openQuotePos + 1);
            if (closeQuotePos != -1) {
                const QByteArray name = content.mid(openQuotePos + 1, closeQuotePos - openQuotePos - 1);
                const QByteArray sourceCode = getSnippet(name);
                if (sourceCode.isEmpty()) {
                    qWarning("Unrecognized name in QC_INCLUDE statement: %s", name.constData());
                } else {
                    content.replace(foundPos, closeQuotePos - foundPos + 1, sourceCode);
                }
            } else {
                qWarning("No closing double quote found for QC_INCLUDE");
            }
        } else {
            qWarning("No opening double quote found for QC_INCLUDE");
        }
        pos = foundPos + 1;
    }

    outFile.write(content);

    return true;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser cmdLineParser;
    const QString appDesc = "Qt Canvas Painter Shader Generator"_L1;
    cmdLineParser.setApplicationDescription(appDesc);
    app.setApplicationVersion(QLatin1String(QT_VERSION_STR));
    cmdLineParser.addHelpOption();
    cmdLineParser.addVersionOption();

    cmdLineParser.addPositionalArgument(QLatin1String("file"),
                                    QObject::tr("Vulkan GLSL source file to preprocess. QC_INCLUDE statements will get replaced with the appropriate source code. "
                                                "The file extension must be .vert or .frag."
                                                ),
                                    QObject::tr("file"));

    QCommandLineOption outputOption({ "o", "output" },
                                     QObject::tr("Output file."),
                                     QObject::tr("filename"));
    cmdLineParser.addOption(outputOption);

    cmdLineParser.process(app);

    if (cmdLineParser.positionalArguments().isEmpty()) {
        cmdLineParser.showHelp();
        return 0;
    }

    if (cmdLineParser.isSet(outputOption)) {
        const QString inputFile = cmdLineParser.positionalArguments().first();
        const QString outputFile = cmdLineParser.value(outputOption);
        if (!process(inputFile, outputFile))
            return 1;
    } else {
        printError("No output file specified");
    }

    return 0;
}

// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QCoreApplication>

#include <QXmlStreamWriter>
#include <qlist.h>

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <QtCore/qfile.h>
#include <QtCore/qdir.h>

#include <QtQuick3DUtils/private/qqsbcollection_p.h>

#include "genshaders.h"

static int generateShaders(QVector<QString> &qsbcFiles,
                           const QDir &outDir,
                           bool verboseOutput,
                           bool dryRun)
{
    GenShaders genShaders;
    if (!genShaders.process(qsbcFiles, outDir, verboseOutput, dryRun))
        return -1;
    return 0;
}

static int writeResourceFile(const QString &resourceFile,
                             const QVector<QString> &qsbcFiles,
                             const QDir &outDir)
{
    if (qsbcFiles.isEmpty())
        return -1;

    const QString outFilename = outDir.canonicalPath() + QDir::separator() + resourceFile;
    QFile outFile(outFilename);
    if (!outFile.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
        qWarning() << "Unable to create output file " << outFilename;
        return -1;
    }

    QXmlStreamWriter writer(&outFile);
    writer.setAutoFormatting(true);
    writer.writeStartElement("RCC");
        writer.writeStartElement("qresource");
        writer.writeAttribute("prefix", "/");
        for (const auto &f : qsbcFiles)
            writer.writeTextElement("file", f);
        writer.writeEndElement();
    writer.writeEndElement();
    outFile.close();

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser cmdLineparser;
    cmdLineparser.setApplicationDescription("Pre-generates particle material shaders for Qt Quick 3D");
    cmdLineparser.addHelpOption();
    // File options
    QCommandLineOption changeDirOption({QChar(u'C'), QLatin1String("directory")},
                                       QLatin1String("Change the working directory"),
                                       QLatin1String("dir"));
    cmdLineparser.addOption(changeDirOption);

    // Debug options
    QCommandLineOption verboseOutputOption({QChar(u'v'), QLatin1String("verbose")}, QLatin1String("Turn on verbose output."));
    cmdLineparser.addOption(verboseOutputOption);

    // Generator options
    QCommandLineOption dryRunOption({QChar(u'n'), QLatin1String("dry-run")}, QLatin1String("Runs as normal, but no files are created."));
    cmdLineparser.addOption(dryRunOption);

    QCommandLineOption outputDirOption({QChar(u'o'), QLatin1String("output-dir")}, QLatin1String("Output directory for generated files."), QLatin1String("file"));
    cmdLineparser.addOption(outputDirOption);

    QCommandLineOption resourceFileOption({QChar(u'r'), QLatin1String("resource-file")}, QLatin1String("Name of generated resource file."), QLatin1String("file"));
    cmdLineparser.addOption(resourceFileOption);

    cmdLineparser.process(a);

    if (cmdLineparser.isSet(changeDirOption)) {
        const auto value = cmdLineparser.value(changeDirOption);
        QFileInfo fi(value);
        if (!fi.isDir()) {
            qWarning("%s : %s - Not a directory", qPrintable(a.applicationName()), qPrintable(value));
            return -1;
        }
        QDir::setCurrent(value);
    }

    QString resourceFile = cmdLineparser.value(resourceFileOption);
    if (resourceFile.isEmpty()) {
        qWarning("%s: resource-file Not set.", qPrintable(a.applicationName()));
        return -1;
    }

    const bool dryRun = cmdLineparser.isSet(dryRunOption);
    const QString &outputPath = cmdLineparser.isSet(outputDirOption) ? cmdLineparser.value(outputDirOption) : QDir::currentPath();
    QDir outDir;
    if (!outputPath.isEmpty() && !dryRun) {
        outDir.setPath(outputPath);
        if (outDir.exists(outputPath) || (!outDir.exists(outputPath) && outDir.mkpath(outputPath))) {
            outDir.setPath(outputPath);
            qDebug("Writing files to %s", qPrintable(outDir.canonicalPath()));
        } else {
            qDebug("Unable to change or create output folder %s", qPrintable(outputPath));
            return -1;
        }
    }

    const bool verboseOutput = cmdLineparser.isSet(verboseOutputOption);

    QVector<QString> qsbcFiles;

    int ret = generateShaders(qsbcFiles, outDir, verboseOutput, dryRun);

    if (ret == 0 && !dryRun)
        writeResourceFile(resourceFile, qsbcFiles, outDir);

    a.exit(ret);
    return ret;
}

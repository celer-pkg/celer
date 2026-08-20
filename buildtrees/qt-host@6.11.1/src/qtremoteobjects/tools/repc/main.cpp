// Copyright (C) 2017-2020 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <qcommandlineoption.h>
#include <qcommandlineparser.h>
#include <qcoreapplication.h>
#include <qfileinfo.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qhashfunctions.h>

#include "cppcodegenerator.h"
#include "repcodegenerator.h"
#include "repparser.h"
#include "utils.h"

#include <cstdio>

#define PROGRAM_NAME  "repc"
#define REPC_VERSION  "1.0.0"

enum Mode {
    InRep = 1,
    InJson = 2,
    OutRep = 4,
    OutSource = 8,
    OutReplica = 16,
    OutMerged = OutSource | OutReplica
};

static const QLatin1String REP("rep");

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    QHashSeed::setDeterministicGlobalSeed();

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QString::fromLatin1(REPC_VERSION));

    QString outputFile;
    QString inputFile;
    int mode = 0;
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("repc tool v%1 (Qt %2).\n")
                                     .arg(QStringLiteral(REPC_VERSION), QString::fromLatin1(QT_VERSION_STR)));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption inputTypeOption(QStringLiteral("i"));
    inputTypeOption.setDescription(QLatin1String("Input file type:\n"
                                                  "rep: replicant template files.\n"
                                                  "json: JSON output from moc of a Qt header file."));
    inputTypeOption.setValueName(QStringLiteral("rep|json"));
    parser.addOption(inputTypeOption);

    QCommandLineOption outputTypeOption(QStringLiteral("o"));
    outputTypeOption.setDescription(QLatin1String("Output file type:\n"
                                                   "source: generates source header. Is incompatible with \"-i src\" option.\n"
                                                   "replica: generates replica header.\n"
                                                   "merged: generates combined replica/source header.\n"
                                                   "rep: generates replicant template file from C++ QOject classes. Is not compatible with \"-i rep\" option."));
    outputTypeOption.setValueName(QStringLiteral("source|replica|merged|rep"));
    parser.addOption(outputTypeOption);

    QCommandLineOption includePathOption(QStringLiteral("I"));
    includePathOption.setDescription(QStringLiteral("Add dir to the include path for header files. This parameter is needed only if the input file type is src (.h file)."));
    includePathOption.setValueName(QStringLiteral("dir"));
    parser.addOption(includePathOption);

    QCommandLineOption alwaysClassOption(QStringLiteral("c"));
    alwaysClassOption.setDescription(QStringLiteral("Always output `class` type for .rep files and never `POD`."));
    parser.addOption(alwaysClassOption);

    QCommandLineOption debugOption(QStringLiteral("d"));
    debugOption.setDescription(QStringLiteral("Print out parsing debug information (for troubleshooting)."));
    parser.addOption(debugOption);

    parser.addPositionalArgument(QStringLiteral("[json-file/rep-file]"),
            QStringLiteral("Input json/rep file to read from, otherwise stdin."));

    parser.addPositionalArgument(QStringLiteral("[rep-file/header-file]"),
            QStringLiteral("Output header/rep file to write to, otherwise stdout."));

    parser.process(app.arguments());

    const QStringList files = parser.positionalArguments();

    if (files.size() > 2) {
        fprintf(stderr, "%s", qPrintable(QLatin1String(PROGRAM_NAME ": Too many input, output files specified: '") + files.join(QStringLiteral("' '")) + QStringLiteral("\'.\n")));
        parser.showHelp(1);
    }

    if (parser.isSet(inputTypeOption)) {
        const QString &inputType = parser.value(inputTypeOption);
        if (inputType == REP)
            mode = InRep;
        else if (inputType == u"json")
            mode = InJson;
        else {
            fprintf(stderr, PROGRAM_NAME ": Unknown input type\"%s\".\n", qPrintable(inputType));
            parser.showHelp(1);
        }
    }

    if (parser.isSet(outputTypeOption)) {
        const QString &outputType = parser.value(outputTypeOption);
        if (outputType == REP)
            mode |= OutRep;
        else if (outputType == u"replica")
            mode |= OutReplica;
        else if (outputType == u"source")
            mode |= OutSource;
        else if (outputType == u"merged")
            mode |= OutMerged;
        else {
            fprintf(stderr, PROGRAM_NAME ": Unknown output type\"%s\".\n", qPrintable(outputType));
            parser.showHelp(1);
        }
    }

    switch (files.size()) {
    case 2:
        outputFile = files.last();
        if (!(mode & (OutRep | OutSource | OutReplica))) {
            // try to figure out the Out mode from file extension
            if (outputFile.endsWith(QLatin1String(".rep")))
                mode |= OutRep;
        }
        Q_FALLTHROUGH();
    case 1:
        inputFile = files.first();
        if (!(mode & (InRep | InJson))) {
            // try to figure out the In mode from file extension
            if (inputFile.endsWith(QLatin1String(".rep")))
                mode |= InRep;
            else
                mode |= InJson;
        }
        break;
    }
    // check mode sanity
    if (!(mode & (InRep | InJson))) {
        fprintf(stderr, PROGRAM_NAME ": Unknown input type, please use -i option to specify one.\n");
        parser.showHelp(1);
    }
    if (!(mode & (OutRep | OutSource | OutReplica))) {
        fprintf(stderr, PROGRAM_NAME ": Unknown output type, please use -o option to specify one.\n");
        parser.showHelp(1);
    }
    if (mode & InRep && mode & OutRep) {
        fprintf(stderr, PROGRAM_NAME ": Invalid input/output type combination, both are rep files.\n");
        parser.showHelp(1);
    }
    if (mode & InJson && mode & OutSource) {
        fprintf(stderr, PROGRAM_NAME ": Invalid input/output type combination, both are source header files.\n");
        parser.showHelp(1);
    }

    QFile input;
    if (inputFile.isEmpty()) {
        inputFile = QStringLiteral("<stdin>");
        if (!input.open(stdin, QIODevice::ReadOnly)) {
            fprintf(stderr, PROGRAM_NAME ": Failed to open stdin for reading: %s\n",
                    qPrintable(input.errorString()));
            return 1;
        }
    } else {
        input.setFileName(inputFile);
        if (!input.open(QIODevice::ReadOnly)) {
            fprintf(stderr, PROGRAM_NAME ": %s: No such file.\n", qPrintable(inputFile));
            return 1;
        }
    }

    QFile output;
    if (outputFile.isEmpty()) {
        if (!output.open(stdout, QIODevice::WriteOnly)) {
            fprintf(stderr, PROGRAM_NAME ": could not open output file '%s': %s.\n",
                    qPrintable(outputFile), qPrintable(output.errorString()));
            return 1;
        }
    } else {
        output.setFileName(outputFile);
        if (!output.open(QIODevice::WriteOnly)) {
            fprintf(stderr, PROGRAM_NAME ": could not open output file '%s': %s.\n",
                    qPrintable(outputFile), qPrintable(output.errorString()));
            return 1;
        }
    }

    if (mode & InJson) {
        QJsonDocument doc(QJsonDocument::fromJson(input.readAll()));
        input.close();
        if (!doc.isObject()) {
            fprintf(stderr, PROGRAM_NAME ": Unable to read json input.\n");
            return 0;
        }

        QJsonObject json = doc.object();

        if (!json.contains(QLatin1String("classes")) || !json[QLatin1String("classes")].isArray()) {
            fprintf(stderr, PROGRAM_NAME ": No QObject classes found.\n");
            return 0;
        }

        QJsonArray classes = json[QLatin1String("classes")].toArray();

        if (mode & OutRep) {
            CppCodeGenerator generator(&output);
            generator.generate(classes, parser.isSet(alwaysClassOption));
        } else {
            Q_ASSERT(mode & OutReplica);
            RepCodeGenerator generator(&output, classList2AST(classes));
            generator.generate(RepCodeGenerator::REPLICA, outputFile);
        }
    } else {
        Q_ASSERT(!(mode & OutRep));
        RepParser repparser(input);
        if (parser.isSet(debugOption))
            repparser.setDebug();
        if (!repparser.parse()) {
            fprintf(stderr, PROGRAM_NAME ": %s:%d: error: %s\n", qPrintable(inputFile), repparser.lineNumber(), qPrintable(repparser.errorString()));
            // if everything is okay and only the input was malformed => remove the output file
            // let's not create an empty file -- make sure the build system tries to run repc again
            // this is the same behavior other code generators exhibit (e.g. flex)
            output.remove();
            return 1;
        }

        input.close();

        RepCodeGenerator generator(&output, repparser.ast());
        if ((mode & OutMerged) == OutMerged)
            generator.generate(RepCodeGenerator::MERGED, outputFile);
        else if (mode & OutReplica)
            generator.generate(RepCodeGenerator::REPLICA, outputFile);
        else if (mode & OutSource)
            generator.generate(RepCodeGenerator::SOURCE, outputFile);
        else {
            fprintf(stderr, PROGRAM_NAME ": Unknown mode.\n");
            return 1;
        }
    }

    output.close();
    return 0;
}

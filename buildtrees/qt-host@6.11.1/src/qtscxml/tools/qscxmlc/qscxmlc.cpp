// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtScxml/private/qscxmlcompiler_p.h>
#include <QtScxml/qscxmltabledata.h>
#include "scxmlcppdumper.h"
#include "qscxmlc.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QStringConverter>

QT_BEGIN_NAMESPACE

enum {
    NoError = 0,
    CommandLineArgumentsError = -1,
    NoInputFilesError = -2,
    CannotOpenInputFileError = -3,
    ParseError = -4,
    CannotOpenOutputHeaderFileError = -5,
    CannotOpenOutputCppFileError = -6,
    ScxmlVerificationError = -7,
    NoTextCodecError = -8
};

int write(TranslationUnit *tu)
{
    QTextStream errs(stderr, QIODevice::WriteOnly);

    QFile outH(tu->outHFileName);
    if (!outH.open(QFile::WriteOnly)) {
        errs << QStringLiteral("Error: cannot open '%1': %2").arg(outH.fileName(), outH.errorString()) << Qt::endl;
        return CannotOpenOutputHeaderFileError;
    }

    QFile outCpp(tu->outCppFileName);
    if (!outCpp.open(QFile::WriteOnly)) {
        errs << QStringLiteral("Error: cannot open '%1': %2").arg(outCpp.fileName(), outCpp.errorString()) << Qt::endl;
        return CannotOpenOutputCppFileError;
    }

    // Make sure it outputs UTF-8, as that is what C++ expects.
    auto utf8 = QStringConverter::encodingForName("UTF-8");
    if (!utf8) {
        errs << QStringLiteral("Error: cannot find a QStringConverter for generating UTF-8.");
        return NoTextCodecError;
    }

    QTextStream h(&outH);
    h.setEncoding(utf8.value());
    h.setGenerateByteOrderMark(true);
    QTextStream c(&outCpp);
    c.setEncoding(utf8.value());
    c.setGenerateByteOrderMark(true);
    CppDumper dumper(h, c);
    dumper.dump(tu);
    h.flush();
    outH.close();
    c.flush();
    outCpp.close();
    return NoError;
}

static void collectAllDocuments(DocumentModel::ScxmlDocument *doc,
                                QList<DocumentModel::ScxmlDocument *> *docs)
{
    docs->append(doc);
    for (DocumentModel::ScxmlDocument *subDoc : std::as_const(doc->allSubDocuments))
        collectAllDocuments(subDoc, docs);
}

int run(const QStringList &arguments)
{
    QCommandLineParser cmdParser;
    QTextStream errs(stderr, QIODevice::WriteOnly);

    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.setApplicationDescription(QCoreApplication::translate("main",
                       "Compiles the given input.scxml file to a header and a cpp file."));

    QCommandLineOption optionNamespace(QLatin1String("namespace"),
                       QCoreApplication::translate("main", "Put generated code into <namespace>."),
                       QCoreApplication::translate("main", "namespace"));
    QCommandLineOption optionOutputBaseName(QStringList() << QLatin1String("o") << QLatin1String("output"),
                       QCoreApplication::translate("main", "Generate <name>.h and <name>.cpp files."),
                       QCoreApplication::translate("main", "name"));
    QCommandLineOption optionOutputHeaderName(QLatin1String("header"),
                       QCoreApplication::translate("main", "Generate <name> for the header file."),
                       QCoreApplication::translate("main", "name"));
    QCommandLineOption optionOutputSourceName(QLatin1String("impl"),
                       QCoreApplication::translate("main", "Generate <name> for the source file."),
                       QCoreApplication::translate("main", "name"));
    QCommandLineOption optionClassName(QLatin1String("classname"),
                       QCoreApplication::translate("main", "Generate <name> for state machine class name."),
                       QCoreApplication::translate("main", "name"));
    QCommandLineOption optionStateMethods(QLatin1String("statemethods"),
                       QCoreApplication::translate("main", "Generate read and notify methods for states"));

    cmdParser.addPositionalArgument(QLatin1String("input"),
                       QCoreApplication::translate("main", "Input SCXML file."));
    cmdParser.addOption(optionNamespace);
    cmdParser.addOption(optionOutputBaseName);
    cmdParser.addOption(optionOutputHeaderName);
    cmdParser.addOption(optionOutputSourceName);
    cmdParser.addOption(optionClassName);
    cmdParser.addOption(optionStateMethods);

    cmdParser.process(arguments);

    const QStringList inputFiles = cmdParser.positionalArguments();

    if (inputFiles.size() < 1) {
        errs << QCoreApplication::translate("main", "Error: no input file.") << Qt::endl;
        cmdParser.showHelp(NoInputFilesError);
    }

    if (inputFiles.size() > 1) {
        errs << QCoreApplication::translate("main", "Error: unexpected argument(s): %1")
                .arg(inputFiles.mid(1).join(QLatin1Char(' '))) << Qt::endl;
        cmdParser.showHelp(NoInputFilesError);
    }

    const QString scxmlFileName = inputFiles.at(0);

    TranslationUnit options;
    options.stateMethods = cmdParser.isSet(optionStateMethods);
    if (cmdParser.isSet(optionNamespace))
        options.namespaceName = cmdParser.value(optionNamespace);
    QString outFileName = cmdParser.value(optionOutputBaseName);
    QString outHFileName = cmdParser.value(optionOutputHeaderName);
    QString outCppFileName = cmdParser.value(optionOutputSourceName);
    QString mainClassName = cmdParser.value(optionClassName);

    if (outFileName.isEmpty())
        outFileName = QFileInfo(scxmlFileName).baseName();
    if (outHFileName.isEmpty())
        outHFileName = outFileName + QLatin1String(".h");
    if (outCppFileName.isEmpty())
        outCppFileName = outFileName + QLatin1String(".cpp");

    QFile file(scxmlFileName);
    if (!file.open(QFile::ReadOnly)) {
        errs << QStringLiteral("Error: cannot open input file %1").arg(scxmlFileName);
        return CannotOpenInputFileError;
    }

    QXmlStreamReader reader(&file);
    QScxmlCompiler compiler(&reader);
    compiler.setFileName(file.fileName());
    compiler.compile();
    if (!compiler.errors().isEmpty()) {
        const auto errors = compiler.errors();
        for (const QScxmlError &error : errors) {
            errs << error.toString() << Qt::endl;
        }
        return ParseError;
    }

    auto mainDoc = QScxmlCompilerPrivate::get(&compiler)->scxmlDocument();
    if (mainDoc == nullptr) {
        Q_ASSERT(!compiler.errors().isEmpty());
        const auto errors = compiler.errors();
        for (const QScxmlError &error : errors) {
            errs << error.toString() << Qt::endl;
        }
        return ScxmlVerificationError;
    }

    if (mainClassName.isEmpty())
        mainClassName = mainDoc->root->name;
    if (mainClassName.isEmpty()) {
        mainClassName = QFileInfo(scxmlFileName).fileName();
        int dot = mainClassName.lastIndexOf(QLatin1Char('.'));
        if (dot != -1)
            mainClassName = mainClassName.left(dot);
    }

    QList<DocumentModel::ScxmlDocument *> docs;
    collectAllDocuments(mainDoc, &docs);

    TranslationUnit tu = options;
    tu.allDocuments = docs;
    tu.scxmlFileName = QFileInfo(file).fileName();
    tu.mainDocument = mainDoc;
    tu.outHFileName = outHFileName;
    tu.outCppFileName = outCppFileName;
    tu.classnameForDocument.insert(mainDoc, mainClassName);

    docs.pop_front();

    for (DocumentModel::ScxmlDocument *doc : std::as_const(docs)) {
        auto name = doc->root->name;
        auto prefix = name;
        if (name.isEmpty()) {
            prefix = QStringLiteral("%1_StateMachine").arg(mainClassName);
            name = prefix;
        }

        int counter = 1;
        while (tu.classnameForDocument.key(name) != nullptr)
            name = QStringLiteral("%1_%2").arg(prefix).arg(++counter);

        tu.classnameForDocument.insert(doc, name);
    }

    return write(&tu);
}

QT_END_NAMESPACE

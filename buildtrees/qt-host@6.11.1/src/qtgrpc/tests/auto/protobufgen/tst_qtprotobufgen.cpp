// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtProtobuf/qtprotobuftypes.h>

#include <QtCore/qdir.h>

#if !QT_CONFIG(process) || defined(CROSSCOMPILING)
#  define SKIP_COMMAND_LINE_TESTS
#endif

#if !defined(SKIP_COMMAND_LINE_TESTS)
#  include <QtCore/qprocess.h>
#endif
#include <QtCore/qstring.h>

#include "protocplugintestcommon.h"

using namespace Qt::StringLiterals;
using namespace ProtocPluginTest;

namespace {
#if !defined(SKIP_COMMAND_LINE_TESTS)
#  ifndef PROTOC_EXECUTABLE
#    error PROTOC_EXECUTABLE definition must be set and point to the valid protoc executable
#  endif

#  ifndef PROTOC_PLUGIN
#    error PROTOC_PLUGIN definition must be set and point to the valid protoc plugin
#  endif
constexpr QLatin1StringView QtprotobufgenPath(PROTOC_PLUGIN);

constexpr QLatin1StringView PluginKey(" --plugin=protoc-gen-qtprotobuf=");
constexpr QLatin1StringView OptKey(" --qtprotobuf_opt=");
constexpr QLatin1StringView OutKey(" --qtprotobuf_out=");
constexpr QLatin1StringView IncludeKey(" -I");
#  ifdef ALLOW_PROTO3_OPTIONAL
const QLatin1StringView allow_proto3_optional(" --experimental_allow_proto3_optional");
#  else
constexpr QLatin1StringView allow_proto3_optional;
#  endif // ALLOW_PROTO3_OPTIONAL

#endif // !defined(SKIP_COMMAND_LINE_TESTS)

constexpr QLatin1StringView ProtocPath(PROTOC_EXECUTABLE);
constexpr QLatin1StringView CmdLineGeneratedDir("cmd_line_generated");

#  ifndef BINARY_DIR
#    error BINARY_DIR definition must be set
#  endif
constexpr QLatin1StringView BinaryDir(BINARY_DIR);

#ifndef CMAKE_GENERATOR_TESTS
#  define CMAKE_GENERATOR_TESTS
#endif
constexpr QLatin1StringView CMakeGeneratorTests(CMAKE_GENERATOR_TESTS);

#ifndef CMAKE_GENERATED_DIR
#  error CMAKE_GENERATED_DIR definition must be set
#endif
constexpr QLatin1StringView CMakeGeneratedDir(CMAKE_GENERATED_DIR);
} // namespace

class qtprotobufgenTest : public ProtocPluginTest::TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    //! Test qt_add_protobuf() cmake function
    void cmakeGenerated_data();
    void cmakeGenerated();

#if !defined(SKIP_COMMAND_LINE_TESTS)
    //! Test command-line call of qtprotobufgen
    void cmdLineGenerated_data();
    void cmdLineGenerated();
    void cmdLineInvalidExportMacro_data();
    void cmdLineInvalidExportMacro();

    void cmdLineMutableGetterConflicts();
#endif

    void cleanupTestCase();
};

void qtprotobufgenTest::initTestCase()
{
    initPaths(BinaryDir, CMakeGeneratedDir, CmdLineGeneratedDir);
    QVERIFY(!cmakeGeneratedPath().isEmpty());
#if !defined(SKIP_COMMAND_LINE_TESTS)
    QVERIFY(!cmdLineGeneratedPath().isEmpty());
    QVERIFY(protocolCompilerAvailableToRun(ProtocPath));
#endif
}

void qtprotobufgenTest::cmakeGenerated_data()
{
    QTest::addColumn<QString>("testName");
    QTest::addColumn<QString>("filePath");

    const QStringList tests = QString(CMakeGeneratorTests).split(','_L1, Qt::SkipEmptyParts);
    for (const auto &testName : tests) {
        QDir testDir(cmakeExpectedResultPath() + '/'_L1 + testName);
        const auto testFiles = scanDirectoryRecursively(testDir);
        for (const auto &testFile : testFiles) {
            auto relativePath = testDir.relativeFilePath(testFile.absoluteFilePath());
            QTest::addRow("%s: %s", testName.toUtf8().constData(),
                          relativePath.toUtf8().constData())
                << testName << relativePath;
        }
    }
}

void qtprotobufgenTest::cmakeGenerated()
{
    QFETCH(QString, testName);
    QFETCH(QString, filePath);
    compareTwoFiles(cmakeExpectedResultPath() + '/'_L1 + testName + '/'_L1 + filePath,
                    cmakeGeneratedPath() + '/'_L1 + testName + '/'_L1 + filePath);
}

#if !defined(SKIP_COMMAND_LINE_TESTS)
void qtprotobufgenTest::cmdLineGenerated_data()
{
    QTest::addColumn<QString>("directory");
    QTest::addColumn<QString>("protoFile");
    QTest::addColumn<QString>("generatorOptions");

    QTest::addRow("no_options") << "no_options"
                                << "qtprotobufgen.proto"
                                << "";

    QTest::addRow("export_macro") << "export_macro"
                                  << "qtprotobufgen.proto"
                                  << "EXPORT_MACRO=TST_QTPROTOBUFGEN_GEN";

    QTest::addRow("generate_package_subfolders") << "generate_package_subfolders"
                                                 << "qtprotobufgen.proto"
                                                 << "GENERATE_PACKAGE_SUBFOLDERS";

    QTest::addRow("copy_comments") << "copy_comments"
                                   << "annotation.proto"
                                   << "COPY_COMMENTS";

    QTest::addRow("extra_namespace") << "extra_namespace"
                                     << "qtprotobufgen.proto"
                                     << "EXTRA_NAMESPACE=MyTopLevelNamespace";

    QTest::addRow("qml") << "qml"
                         << "qtprotobufgen.proto"
                         << "QML";

    QTest::addRow("qml_uri") << "qml_uri"
                             << "qtprotobufgen.proto"
                             << "QML;QML_URI=my.test.uri";

    QTest::addRow("qml_uri_export_macro")
        << "qml_uri_export_macro"
        << "qtprotobufgen.proto"
        << "QML;QML_URI=my.test.uri;EXPORT_MACRO=TST_QTPROTOBUFGEN_GEN";

    QTest::addRow("qml_uri_export_macro_generate_package_subfolders")
        << "qml_uri_export_macro_generate_package_subfolders"
        << "qtprotobufgen.proto"
        << "QML;QML_URI=my.test.uri;EXPORT_MACRO=TST_QTPROTOBUFGEN_GEN;GENERATE_PACKAGE_SUBFOLDERS";

    QTest::addRow("export_macro_custom_file_name")
        << "export_macro_custom_file_name"
        << "qtprotobufgen.proto"
        << "EXPORT_MACRO=EXPORT_MACRO_WITH_FILE:custom_file_name";

    QTest::addRow("export_macro_custom_file_name_force_generate")
        << "export_macro_custom_file_name_force_generate"
        << "qtprotobufgen.proto"
        << "EXPORT_MACRO=EXPORT_MACRO_WITH_FILE:custom_file_name.hpp:true";

    QTest::addRow("export_macro_custom_file_name_skip_generate")
        << "export_macro_custom_file_name_skip_generate"
        << "qtprotobufgen.proto"
        << "EXPORT_MACRO=EXPORT_MACRO_WITH_FILE:custom_file_name.hxx:false";

    QTest::addRow("header_guard_filename")
        << "header_guard_filename"
        << "qtprotobufgenminimal.proto"
        << "HEADER_GUARD=filename";

    QTest::addRow("header_guard_pragma")
        << "header_guard_pragma"
        << "qtprotobufgenminimal.proto"
        << "HEADER_GUARD=pragma";

    QTest::addRow("non_final_messages")
        << "non_final_messages"
        << "qtprotobufgenminimal.proto"
        << "GENERATE_NON_FINAL_MESSAGES";
}

void qtprotobufgenTest::cmdLineGenerated()
{
    QFETCH(QString, directory);
    QFETCH(QString, protoFile);
    QFETCH(QString, generatorOptions);

    const QString fullProtoFilePath(expectedResultPath() + '/'_L1 + protoFile);
    QVERIFY2(QFile::exists(fullProtoFilePath),
             qPrintable("Input .proto scheme "_L1 + fullProtoFilePath + " doesn't exists"_L1));

    QDir outputDirectory(cmdLineGeneratedPath());
    if (!outputDirectory.exists(directory))
        outputDirectory.mkdir(directory);
    outputDirectory.cd(directory);

    QProcess process;
    process.setWorkingDirectory(cmdLineGeneratedPath());
    process.startCommand(ProtocPath + PluginKey + QtprotobufgenPath + OptKey + generatorOptions
                         + OutKey + outputDirectory.absolutePath() + IncludeKey
                         + expectedResultPath() + ' '_L1 + fullProtoFilePath
                         + allow_proto3_optional);

    QVERIFY2(process.waitForStarted(), qPrintable(msgProcessStartFailed(process)));
    if (!process.waitForFinished()) {
        process.kill();
        QFAIL(qPrintable(msgProcessTimeout(process)));
    }
    QVERIFY2(process.exitStatus() == QProcess::NormalExit, qPrintable(msgProcessCrashed(process)));
    QVERIFY2(process.exitCode() == 0, qPrintable(msgProcessFailed(process)));

    QDir expectedResultDir(cmdLineExpectedResultPath() + '/'_L1 + directory);
    const auto generatedFileList = scanDirectoryRecursively(outputDirectory);
    const auto expectedFileList = scanDirectoryRecursively(expectedResultDir);

    QCOMPARE_EQ(relativePaths(outputDirectory, generatedFileList),
                relativePaths(expectedResultDir, expectedFileList));

    for (qsizetype i = 0; i < expectedFileList.size(); ++i) {
        compareTwoFiles(generatedFileList.at(i).absoluteFilePath(),
                        expectedFileList.at(i).absoluteFilePath());
    }
}

void qtprotobufgenTest::cmdLineInvalidExportMacro_data()
{
    QTest::addColumn<QString>("exportMacro");
    QTest::addColumn<int>("result");

    QTest::addRow("contains_dash") << "TST_QTPROTOBUFGEN-FAIL" << 1;
    QTest::addRow("contains_number_first") << "1Not_ALLoWeD" << 1;
}

void qtprotobufgenTest::cmdLineInvalidExportMacro()
{
    QFETCH(QString, exportMacro);
    QFETCH(int, result);

    static constexpr QLatin1StringView directory("invalid_export_macro");
    QDir outputDirectory(cmdLineGeneratedPath());
    if (!outputDirectory.exists(directory))
        outputDirectory.mkdir(directory);
    outputDirectory.cd(directory);
    QString exportMacroCmd = "EXPORT_MACRO=" + exportMacro;

    QProcess process;
    process.setWorkingDirectory(cmdLineGeneratedPath());
    process.startCommand(ProtocPath + QString(" ") + PluginKey + QtprotobufgenPath
                         + OptKey + exportMacroCmd
                         + OutKey + outputDirectory.absolutePath()
                         + IncludeKey + expectedResultPath() +
                         + " " + expectedResultPath() + "/qtprotobufgen.proto"
                         + allow_proto3_optional);
    QVERIFY2(process.waitForStarted(), msgProcessStartFailed(process).constData());
    if (!process.waitForFinished()) {
        process.kill();
        QFAIL(msgProcessTimeout(process).constData());
    }
    QVERIFY2(process.exitStatus() == QProcess::NormalExit, msgProcessCrashed(process).constData());
    QVERIFY2(process.exitCode() == result, msgProcessFailed(process).constData());
}

void qtprotobufgenTest::cmdLineMutableGetterConflicts()
{
    static constexpr QLatin1StringView directory("invalid_export_macro");
    QDir outputDirectory(cmdLineGeneratedPath());
    if (!outputDirectory.exists(directory))
        outputDirectory.mkdir(directory);
    outputDirectory.cd(directory);

    QProcess process;
    process.setWorkingDirectory(cmdLineGeneratedPath());
    process.startCommand(ProtocPath + QString(" ") + PluginKey + QtprotobufgenPath
                         + OutKey + outputDirectory.absolutePath()
                         + IncludeKey + expectedResultPath()
                         + " " + expectedResultPath()
                         + "/qtprotobufgen_mutable_getter_clashing.proto");
    QVERIFY2(process.waitForStarted(), msgProcessStartFailed(process).constData());
    if (!process.waitForFinished()) {
        process.kill();
        QFAIL(msgProcessTimeout(process).constData());
    }
    QVERIFY2(process.exitStatus() == QProcess::NormalExit, msgProcessCrashed(process).constData());
    QVERIFY2(process.exitCode() == 1, msgProcessFailed(process).constData());
}

#endif // !defined(SKIP_COMMAND_LINE_TESTS)

void qtprotobufgenTest::cleanupTestCase()
{
    QVERIFY(cleanupTestData());
}

QTEST_MAIN(qtprotobufgenTest)
#include "tst_qtprotobufgen.moc"

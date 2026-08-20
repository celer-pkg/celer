// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/qtest.h>

#include <QtProtobuf/qtprotobuftypes.h>

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
constexpr QLatin1StringView QtgrpcgenPath(PROTOC_PLUGIN);

constexpr QLatin1StringView PluginKey(" --plugin=protoc-gen-qtgrpc=");
constexpr QLatin1StringView OptKey(" --qtgrpc_opt=");
constexpr QLatin1StringView OutKey(" --qtgrpc_out=");
constexpr QLatin1StringView IncludeKey(" -I");
#endif

constexpr QLatin1StringView ProtocPath(PROTOC_EXECUTABLE);
constexpr QLatin1StringView CmdLineGeneratedDir("cmd_line_generated");

#ifndef BINARY_DIR
#  error BINARY_DIR definition must be set
#endif
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

class qtgrpcgenTest : public ProtocPluginTest::TestBase
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    //! Test qt_add_grpc() cmake function
    void cmakeGenerated_data();
    void cmakeGenerated();

#if !defined(SKIP_COMMAND_LINE_TESTS)
    //! Test command-line call of qtgrpcgen
    void cmdLineGenerated_data();
    void cmdLineGenerated();
#endif

    void cleanupTestCase();
};

void qtgrpcgenTest::initTestCase()
{
    initPaths(BinaryDir, CMakeGeneratedDir, CmdLineGeneratedDir);
}

void qtgrpcgenTest::cmakeGenerated_data()
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

void qtgrpcgenTest::cmakeGenerated()
{
    QFETCH(QString, testName);
    QFETCH(QString, filePath);
    compareTwoFiles(cmakeExpectedResultPath() + '/'_L1 + testName + '/'_L1 + filePath,
                    cmakeGeneratedPath() + '/'_L1 + testName + '/'_L1 + filePath);
}

#if !defined(SKIP_COMMAND_LINE_TESTS)
void qtgrpcgenTest::cmdLineGenerated_data()
{
    QTest::addColumn<QString>("directory");
    QTest::addColumn<QString>("protoFile");
    QTest::addColumn<QString>("generatorOptions");

    QTest::addRow("no_options") << "no_options"
                                << "qtgrpcgen.proto"
                                << "";

    QTest::addRow("header_guard_filename") << "header_guard_filename"
                                           << "qtgrpcgen.proto"
                                           << "HEADER_GUARD=filename";

    QTest::addRow("header_guard_pragma") << "header_guard_pragma"
                                         << "qtgrpcgen.proto"
                                         << "HEADER_GUARD=pragma";

    QTest::addRow("qml") << "qml"
                         << "qtgrpcgen.proto"
                         << "QML";

    QTest::addRow("qml_uri") << "qml_uri"
                             << "qtgrpcgen.proto"
                             << "QML;QML_URI=my.test.uri";

    QTest::addRow("qml_uri_export_macro") << "qml_uri_export_macro"
                                          << "qtgrpcgen.proto"
                                          << "QML;QML_URI=my.test.uri;EXPORT_MACRO=TST_QTGRPC_GEN";

    QTest::addRow("qml_uri_export_macro_generate_package_subfolders")
        << "qml_uri_export_macro_generate_package_subfolders"
        << "qtgrpcgen.proto"
        << "QML;QML_URI=my.test.uri;EXPORT_MACRO=TST_QTGRPC_GEN;GENERATE_PACKAGE_SUBFOLDERS";

    QTest::addRow("export_macro_custom_file_name")
        << "export_macro_custom_file_name"
        << "qtgrpcgen.proto"
        << "EXPORT_MACRO=EXPORT_MACRO_WITH_FILE:custom_file_name";

    QTest::addRow("export_macro_custom_file_name_force_generate")
        << "export_macro_custom_file_name_force_generate"
        << "qtgrpcgen.proto"
        << "EXPORT_MACRO=EXPORT_MACRO_WITH_FILE:custom_file_name.hpp:true";

    QTest::addRow("export_macro_custom_file_name_skip_generate")
        << "export_macro_custom_file_name_skip_generate"
        << "qtgrpcgen.proto"
        << "EXPORT_MACRO=EXPORT_MACRO_WITH_FILE:custom_file_name.hxx:false";
}

void qtgrpcgenTest::cmdLineGenerated()
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
    process.startCommand(ProtocPath + PluginKey + QtgrpcgenPath + OptKey + generatorOptions + OutKey
                         + outputDirectory.absolutePath() + IncludeKey + expectedResultPath()
                         + ' '_L1 + fullProtoFilePath);

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
#endif

void qtgrpcgenTest::cleanupTestCase()
{
    cleanupTestData();
}

QTEST_MAIN(qtgrpcgenTest)
#include "tst_qtgrpcgen.moc"

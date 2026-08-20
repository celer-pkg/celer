// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qlibraryinfo.h>
#include <QtQml/qqml.h>
#include <QtTest/qtest.h>

class tst_generate_qmlls_ini : public QObject
{
    Q_OBJECT
private slots:
    void qmllsIniAreCorrect_data();
    void qmllsIniAreCorrect();

    void qmllsBuildIni_data();
    void qmllsBuildIni();
};

using namespace Qt::StringLiterals;

#ifndef SOURCE_DIRECTORY
#  define SOURCE_DIRECTORY u"invalid_source_directory"_s
#endif
#ifndef BUILD_DIRECTORY
#  define BUILD_DIRECTORY u"invalid_build_directory"_s
#endif

static QString contentOf(const QString &fileName)
{
    auto file = QFile(fileName);
    [&file] {
        QVERIFY(file.exists());
        QVERIFY(file.open(QFile::ReadOnly | QFile::Text));
    }();
    return QString::fromUtf8(file.readAll());
}

void tst_generate_qmlls_ini::qmllsIniAreCorrect_data()
{
    QTest::addColumn<QString>("folder");
    QTest::addColumn<QStringList>("expectedBuildDirs");
    QTest::addColumn<QString>("expectedNoCMakeCalls");
    QTest::addColumn<QString>("expectedDocDir");
    QTest::addColumn<QStringList>("expectedImportPaths");

    QDir source(SOURCE_DIRECTORY);
    QDir build(BUILD_DIRECTORY);
    if (!source.exists()) {
        QSKIP(u"Cannot find source directory '%1', skipping test..."_s.arg(SOURCE_DIRECTORY)
                      .toLatin1());
    }

    const QString &docPath = QLibraryInfo::path(QLibraryInfo::DocumentationPath);
    const QStringList defaultImportPaths{ QLibraryInfo::path(QLibraryInfo::QmlImportsPath) };
    const QString noCMakeCalls = "false"_L1;

    QTest::addRow("subfolders") << source.absolutePath()
                                << QStringList{ build.absolutePath(),
                                                QDir(build.absolutePath().append(
                                                             "/qml/hello/subfolders"_L1))
                                                        .absolutePath() }
                                << noCMakeCalls << docPath << defaultImportPaths;

    {
        QDir sourceSubfolder = source;
        QVERIFY(sourceSubfolder.cd(u"SomeSubfolder"_s));
        QTest::addRow("subfolders2")
                << sourceSubfolder.absolutePath()
                << QStringList{ build.absoluteFilePath(u"SomeSubfolder/qml/Some/Sub/Folder"_s) }
                << noCMakeCalls << docPath << defaultImportPaths;
    }

    {
        QDir dottedUriSubfolder = source;
        QVERIFY(dottedUriSubfolder.cd(u"Dotted"_s));
        QVERIFY(dottedUriSubfolder.cd(u"Uri"_s));
        QTest::addRow("dotted-uri")
                << dottedUriSubfolder.absolutePath() << QStringList{ build.absolutePath() }
                << noCMakeCalls << docPath << defaultImportPaths;
    }
    {
        QDir dottedUriSubfolder = source;
        QVERIFY(dottedUriSubfolder.cd(u"Dotted"_s));
        QVERIFY(dottedUriSubfolder.cd(u"Uri"_s));
        QVERIFY(dottedUriSubfolder.cd(u"Hello"_s));
        QVERIFY(dottedUriSubfolder.cd(u"World"_s));

        QTest::addRow("dotted-uri2")
                << dottedUriSubfolder.absolutePath() << QStringList{ build.absolutePath() }
                << noCMakeCalls << docPath << defaultImportPaths;
    }
    {
        QDir dottedUriSubfolder = source;
        QVERIFY(dottedUriSubfolder.cd(u"ModuleWithDependency"_s));
        QVERIFY(dottedUriSubfolder.cd(u"MyModule"_s));
        QTest::addRow("module-with-dependency")
                << dottedUriSubfolder.absolutePath() << QStringList{ build.absoluteFilePath(u"ModuleWithDependency"_s), }
                << noCMakeCalls << docPath << QStringList { build.absoluteFilePath(u"Dependency"_s) } + defaultImportPaths;
    }
    {
        QDir quotesInPath = source;
        QVERIFY(quotesInPath.cd(u"quotesInPath"_s));
        QTest::addRow("quotes-in-path")
                << quotesInPath.absolutePath() << QStringList{ build.absolutePath(), }
                << noCMakeCalls << docPath << QStringList {R"(\"hello\"\"world\")"_L1 } + defaultImportPaths;
    }
    {
        QDir withoutCMakeBuilds = source;
        QVERIFY(withoutCMakeBuilds.cd(u"WithoutCMakeBuilds"_s));
        QTest::addRow("without-cmake-calls")
                << withoutCMakeBuilds.absolutePath() << QStringList{ build.absolutePath(), }
                << u"true"_s << docPath << defaultImportPaths;
    }
    {
        QDir wrongOutput = source;
        QVERIFY(wrongOutput.cd(u"WrongOutput"_s));
        QTest::addRow("wrong-output")
                << wrongOutput.absolutePath() << QStringList{ build.filePath("WrongOutput"), }
                << noCMakeCalls << docPath << defaultImportPaths;
    }
}

void tst_generate_qmlls_ini::qmllsIniAreCorrect()
{
    QFETCH(QString, folder);
    QFETCH(QStringList, expectedBuildDirs);
    QFETCH(QString, expectedNoCMakeCalls);
    QFETCH(QString, expectedDocDir);
    QFETCH(QStringList, expectedImportPaths);

    static constexpr QLatin1String qmllsIniName = ".qmlls.ini"_L1;
    static constexpr QLatin1String qmllsIniTemplate = R"([General]
buildDir="%1"
no-cmake-calls=%2
docDir=%3
importPaths="%4"
)"_L1;

    const QString iniContent = contentOf(QString(folder).append("/"_L1).append(qmllsIniName));
    QCOMPARE(iniContent,
             qmllsIniTemplate.arg(expectedBuildDirs.join(QDir::listSeparator()),
                                  expectedNoCMakeCalls, expectedDocDir,
                                  expectedImportPaths.join(QDir::listSeparator())));
}

void tst_generate_qmlls_ini::qmllsBuildIni_data()
{
    QTest::addColumn<QString>("expectedSource");
    QTest::addColumn<QStringList>("expectedImportPaths");
    QTest::addColumn<QStringList>("expectedResourceFiles");

    QDir build(BUILD_DIRECTORY);
    QVERIFY(build.exists());

    QDir source(SOURCE_DIRECTORY);
    if (!source.exists()) {
        QSKIP(u"Cannot find source directory '%1', skipping test..."_s.arg(SOURCE_DIRECTORY)
                      .toLatin1());
    }

    const QString pathOfCurrentModule = build.absolutePath();

    QVERIFY(source.cd("QmllsBuildIni"_L1));
    QVERIFY(build.cd("QmllsBuildIni"_L1));

    QTest::addRow("normal") << source.absolutePath()
                            << QStringList{ build.absoluteFilePath("qml2"_L1),
                                            build.absoluteFilePath("qml"_L1),
                                            build.absoluteFilePath("qml3/MyModule3"_L1),
                                            pathOfCurrentModule,
                                            build.absolutePath(),
                                            build.absoluteFilePath(QLibraryInfo::path(
                                                    QLibraryInfo::QmlImportsPath)) }
                            << QStringList{
                                   build.absoluteFilePath(".qt/rcc/qmake_QmllsBuildIni.qrc"_L1),
                                   build.absoluteFilePath(".qt/rcc/QmllsBuildIni_raw_qml_0.qrc"_L1)
                               };

    QVERIFY(source.cd("../ImportPathOrderStudy/MyApp"_L1));
    QVERIFY(build.cd("../ImportPathOrderStudy/MyApp"_L1));

    QTest::addRow("noResourceFilesFromOtherModules")
            << source.absolutePath()
            << QStringList{ pathOfCurrentModule + "/A"_L1, pathOfCurrentModule + "/B"_L1,
               build.absolutePath(), pathOfCurrentModule,
               QLibraryInfo::path(QLibraryInfo::QmlImportsPath), }
            << QStringList{ build.absoluteFilePath(".qt/rcc/qmake_ImportPathOrderStudy.qrc"_L1),
               build.absoluteFilePath(
                   ".qt/rcc/appImportPathOrderStudy_raw_qml_0.qrc"_L1), };
}

void tst_generate_qmlls_ini::qmllsBuildIni()
{
    QFETCH(QString, expectedSource);
    QFETCH(QStringList, expectedImportPaths);
    QFETCH(QStringList, expectedResourceFiles);

    static constexpr QLatin1String qmllsBuildIniPath = ".qt/.qmlls.build.ini"_L1;

    QDir build(BUILD_DIRECTORY);
    QVERIFY(build.exists());

    const QString content = contentOf(build.filePath(qmllsBuildIniPath));

    static constexpr QLatin1String expectedContent = R"([%1]
importPaths="%2"
resourceFiles="%3"
)"_L1;
    QVERIFY(content.contains(
            expectedContent.arg(expectedSource.replace("/"_L1, "<SLASH>"_L1),
                                expectedImportPaths.join(QDir::listSeparator()),
                                expectedResourceFiles.join(QDir::listSeparator()))));
}

QTEST_MAIN(tst_generate_qmlls_ini)

#include "main.moc"

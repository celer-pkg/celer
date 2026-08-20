// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtCore/qstring.h>
#include <QtCore/qprocess.h>
#include <QtCore/qlibraryinfo.h>

using namespace Qt::StringLiterals;

class tst_qmlcontextpropertydump : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_qmlcontextpropertydump();

private slots:
    void dump();

private:
    QString m_executablePath;
};

tst_qmlcontextpropertydump::tst_qmlcontextpropertydump() : QQmlDataTest(QT_QMLTEST_DATADIR)
{
    m_executablePath =
            QLibraryInfo::path(QLibraryInfo::BinariesPath).append("/qmlcontextpropertydump"_L1);

#ifdef Q_OS_WIN
    m_executablePath.append(".exe"_L1);
#endif
    if (!QFileInfo::exists(m_executablePath))
        qWarning() << "executable not found (looked for %0)"_L1.arg(m_executablePath);
}

void tst_qmlcontextpropertydump::dump()
{
    QProcess process;
    process.setProgram(m_executablePath);
    QTemporaryDir output;
    QVERIFY(output.isValid());
    process.setArguments({ "-s"_L1, testFile("ContextProperties"_L1), "-b"_L1, output.path() });

    process.start();
    QVERIFY(process.waitForFinished());
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), 0);

    QFile outputFile(output.filePath(".qt/contextPropertyDump.ini"));
    QVERIFY(outputFile.open(QFile::ReadOnly | QFile::Text));

    const QString outputFileContent = outputFile.readAll();
    const QString expectedOutput =
            R"([cachedHeuristicList]
1\name=myContextProperty1
2\name=myContextProperty2
size=2

[property_myContextProperty1]
1\fileName=%1/src/someFile.cpp
1\sourceLocation="324,18,14,45"
size=1

[property_myContextProperty2]
1\fileName=%1/src/someFile.cpp
1\sourceLocation="438,18,16,19"
size=1
)"_L1.arg(testFile("ContextProperties"_L1));

    {
        QFile expectedOutputFile(output.filePath("expected.ini"));
        QVERIFY(expectedOutputFile.open(QFile::WriteOnly | QFile::Text));
        expectedOutputFile.write(expectedOutput.toUtf8());
    }

    QCOMPARE(outputFileContent, expectedOutput);
}

QTEST_GUILESS_MAIN(tst_qmlcontextpropertydump)
#include "tst_qmlcontextpropertydump.moc"

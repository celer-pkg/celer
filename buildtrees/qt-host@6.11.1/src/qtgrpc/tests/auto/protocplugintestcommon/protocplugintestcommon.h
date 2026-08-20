// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef PROTOCPLUGINTESTCOMMON_H
#define PROTOCPLUGINTESTCOMMON_H

#include <QtTest/qtest.h>

#include <QtCore/qdir.h>
#include <QtCore/qstring.h>

QT_FORWARD_DECLARE_CLASS(QProcess)

namespace ProtocPluginTest {
QByteArray msgCannotReadFile(const QFile &file);
#if QT_CONFIG(process)
QByteArray msgProcessStartFailed(const QProcess &p);
QByteArray msgProcessTimeout(const QProcess &p);
QByteArray msgProcessCrashed(QProcess &p);
QByteArray msgProcessFailed(QProcess &p);
bool protocolCompilerAvailableToRun(const QString &protocPath);
#endif // QT_CONFIG(process)

void compareTwoFiles(const QString &expectedFileName, const QString &actualFileName);
void cleanFolder(const QString &folderName);
QFileInfoList scanDirectoryRecursively(const QDir &dir);
QStringList relativePaths(const QDir &dir, const QFileInfoList &files);

class TestBase : public QObject
{
    Q_OBJECT
public:
    TestBase(QObject *parent = nullptr);

protected:
    void initPaths(const QLatin1StringView binaryDir, const QLatin1StringView cmakeGeneratedDir,
                   const QLatin1StringView cmdLineGeneratedDir);

    const QString &expectedResultPath() const;
    const QString &cmakeGeneratedPath() const;
    const QString &cmakeExpectedResultPath() const;
#if QT_CONFIG(process)
    const QString &cmdLineGeneratedPath() const;
    const QString &cmdLineExpectedResultPath() const;
#endif

    bool cleanupTestData() const;

private:
    // Copy test results back to source directory if COPY_TEST_RESULTS environment variable is
    // set to 'true'. Useful when changing the generator to update the test data.
    bool m_copyTestResults = false;

    QString m_expectedResultPath;
    QString m_cmakeExpectedResultPath;
    QString m_cmakeGeneratedPath;
#if QT_CONFIG(process)
    QString m_cmdLineExpectedResultPath;
    QString m_cmdLineGeneratedPath;
#endif
};
} // namespace ProtocPluginTest

#endif // PROTOCPLUGINTESTCOMMON_H

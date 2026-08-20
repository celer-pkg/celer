// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <QDebug>
#include <QString>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

#ifdef Q_OS_ANDROID
#define LOCAL_SOCKET "localabstract"
#else
#define LOCAL_SOCKET  "local"
#endif

namespace TestUtils {

QString subFolder;
QString rootFolder;

bool init(const QString &folder)
{
    if (!TestUtils::rootFolder.isEmpty())
        return true;

#ifdef Q_OS_ANDROID
    // All libraries are at located at the native libraries folder
    return true;
#endif

    const auto appPath = QCoreApplication::applicationDirPath();
    auto dir = QDir(appPath);
    while (dir.dirName() != folder) {
        TestUtils::subFolder.prepend(dir.dirName()).prepend('/');
        if (!dir.cdUp())
            return false;
    }
    dir.cdUp();
    TestUtils::rootFolder = dir.absolutePath();
    return true;
}

QString findExecutable(const QString &executableName, const QString &folder)
{
    const auto path = QStandardPaths::findExecutable(executableName, {TestUtils::rootFolder + folder + TestUtils::subFolder});
    if (!path.isEmpty()) {
        return path;
    }

    qWarning() << "Could not find executable:" << executableName << "in " << TestUtils::rootFolder + folder + TestUtils::subFolder;
    return QString();
}

}

#endif // TESTUTILS_H

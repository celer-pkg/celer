// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QCoreApplication>
#include <QtQuickTest/quicktest.h>
#include <QtQml/QQmlEngine>

static void initializeLibraryPath()
{
#if QT_CONFIG(library)
    // Set custom path since CI doesn't install test plugins
#ifdef Q_OS_WIN
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../../plugins"));
#else
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../plugins"));
#endif
#endif
    const QByteArray qmlPath = QCoreApplication::applicationDirPath().toLatin1() +
            "/../../../qml";
    qputenv("QML_IMPORT_PATH", qmlPath);
}

Q_COREAPP_STARTUP_FUNCTION(initializeLibraryPath)

QUICK_TEST_MAIN(declarative_ui)

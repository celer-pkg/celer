// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlapplicationengine.h>
#include <QtQml/qqmlcomponent.h>
#include <QtQml/qqmlengine.h>

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qtemporarydir.h>
#include <QtCore/qtenvironmentvariables.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine0; // add another engine to cause some trouble

    if (qEnvironmentVariableIsSet("TEST_MKDIR_RMDIR")) {
        QQmlComponent c(&engine0, QUrl::fromLocalFile(QLatin1String(argv[argc - 1])));
        if (!c.isReady())
            qFatal("%s", qPrintable(c.errorString()));
        QScopedPointer<QObject> o(c.create());
        if (o.isNull())
            qFatal("Failed to create object");

        QTemporaryDir tmpdir;
        if (!QDir().mkpath(tmpdir.filePath("foo")))
            qFatal("mkdir failed");
        if (!QDir().rmpath(tmpdir.filePath("foo")))
            qFatal("rmdir failed");
        qDebug("mkdir rmdir ok");
        return app.exec();
    } else {
        QQmlApplicationEngine engine1;
        engine1.load(QUrl::fromLocalFile(QLatin1String(argv[argc - 1])));
        return app.exec();
    }

}


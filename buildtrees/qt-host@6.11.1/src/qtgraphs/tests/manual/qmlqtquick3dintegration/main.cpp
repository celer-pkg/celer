// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qapplication.h"
#include "videodata.h"

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQuick/qquickview.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQuickView viewer;

    // The following are needed to make examples run without having to install the
    // module in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#endif
    viewer.engine()->addImportPath(
        extraImportPath.arg(QGuiApplication::applicationDirPath(), QString::fromLatin1("qml")));

    VideoData dataGenerator;
    viewer.rootContext()->setContextProperty("dataGenerator", &dataGenerator);

    viewer.setTitle(QStringLiteral("Live Data and QtQuick3D Integration"));
    viewer.setSource(QUrl("qrc:/qml/qmlqtquick3dintegration/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.setColor("black");
    viewer.show();

    return app.exec();
}

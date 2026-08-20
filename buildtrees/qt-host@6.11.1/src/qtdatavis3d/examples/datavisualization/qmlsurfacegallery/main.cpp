// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

//! [0]
#include <QtDataVisualization/qutils.h>
//! [0]

#include <QtGui/qguiapplication.h>
#include <QtQuick/qquickview.h>
#include <QtQml/qqmlengine.h>

#ifdef QMAKE_BUILD
#include "datasource.h"
#endif

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    qputenv("QSG_RHI_BACKEND", "opengl");
    QGuiApplication app(argc, argv);

#ifdef QMAKE_BUILD
    qmlRegisterType<DataSource>("SurfaceGallery", 1, 0, "DataSource");
#endif

    QQuickView viewer;

    //! [1]
    // Enable antialiasing in direct rendering mode
    viewer.setFormat(qDefaultSurfaceFormat(true));
    //! [1]

    // The following are needed to make examples run without having to install the module
    // in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(u"%1/../../../../%2"_s);
#else
    QString extraImportPath(u"%1/../../../%2"_s);
#endif
    viewer.engine()->addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(),
                                                       u"qml"_s));
    QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);

    viewer.setTitle(u"Surface Graph Gallery"_s);

    viewer.setSource(QUrl(u"qrc:/qml/qmlsurfacegallery/main.qml"_s));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.show();

    return app.exec();
}

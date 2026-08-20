// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QApplication>
#include <QDir>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include "resultsio.h"

#if USE_CHARTS
#include "chartsdatasource.h"
#else
#include "datasource.h"
#endif

int main(int argc, char *argv[])
{
    // Qt Charts uses Qt Graphics View Framework for drawing, therefore QApplication must be used.
    QApplication app(argc, argv);

    QQuickView viewer;
    viewer.setMinimumSize({600, 400});

// The following are needed to make examples run without having to install the module
// in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#endif
    viewer.engine()->addImportPath(
        extraImportPath.arg(QGuiApplication::applicationDirPath(), QString::fromLatin1("qml")));
    QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);

    int frame = 0;
    qreal time = QDateTime::currentMSecsSinceEpoch();

    QObject::connect(&viewer, &QQuickView::frameSwapped, &viewer, [&]() {
        frame++;
        if (QDateTime::currentMSecsSinceEpoch() >= time + 1000) {
            time = QDateTime::currentMSecsSinceEpoch();
            viewer.rootObject()->setProperty("fps", frame);
            frame = 0;
        }
    });

#if USE_CHARTS
    ChartsDataSource chartsDataSource(&viewer);
    viewer.rootContext()->setContextProperty("chartsDataSource", &chartsDataSource);
#else
    DataSource dataSource(&viewer);
    viewer.rootContext()->setContextProperty("dataSource", &dataSource);
#endif

    ResultsIO resultsIO(&viewer);
    viewer.rootContext()->setContextProperty("resultsIO", &resultsIO);

    viewer.setTitle(QStringLiteral("Benchmark"));
    viewer.setSource(QUrl("qrc:/Main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.show();

    return app.exec();
}

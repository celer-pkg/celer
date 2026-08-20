// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGui/QGuiApplication>
#include <QtGraphs/QLineSeries>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView viewer;

// The following are needed to make examples run without having to install the module
// in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../%2"));
#endif
    viewer.engine()->addImportPath(
        extraImportPath.arg(QGuiApplication::applicationDirPath(), QString::fromLatin1("qml")));
    QObject::connect(viewer.engine(), &QQmlEngine::quit, &viewer, &QWindow::close);

    viewer.setTitle(QStringLiteral("QML Graphs 2D Lines test"));

    QLineSeries *series = new QLineSeries();
    series->append(0, 2);
    series->append(2, 4);
    series->append(3, std::numeric_limits<qreal>::quiet_NaN());
    series->append(4, 1);
    series->append(5, 0);
    series->setLineStyle(QLineSeries::LineStyle::StepCenter);
    series->setColor(Qt::red);
    viewer.rootContext()->setContextProperty("cppLineSeries", series);

    viewer.setSource(QUrl("qrc:/qml/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.show();

    return app.exec();
}

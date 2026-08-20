// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGui/QGuiApplication>
#include <QtCore/QDir>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtGraphs/QPieSeries>
#include <QtGraphs/QPieSlice>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView viewer;

    // The following are needed to make examples run without having to install the module
    // in desktop environments.
#ifdef Q_OS_WIN
    QString extraImportPath(QStringLiteral("%1/../../../../%2"));
#else
    QString extraImportPath(QStringLiteral("%1/../../../%2"));
#endif
    viewer.engine()->addImportPath(extraImportPath.arg(QGuiApplication::applicationDirPath(),
                                      QString::fromLatin1("qml")));

    viewer.setTitle(QStringLiteral("QML SubPieSlices"));

    QPieSeries *series = new QPieSeries();
    QPieSlice* slice1 = new QPieSlice("salmon", 1);
    slice1->setLabelVisible(true);
    slice1->setColor(slice1->label());
    {
        QPieSlice* subSlice11 = new QPieSlice("lightgrey", 3);
        subSlice11->setLabelVisible(true);
        subSlice11->setColor(subSlice11->label());
        slice1->append(subSlice11);
        QPieSlice* subSlice12 = new QPieSlice("yellow", 5);
        subSlice12->setLabelVisible(true);
        subSlice12->setColor(subSlice12->label());
        slice1->append(subSlice12);
    }
    series->append(slice1);
    QPieSlice* slice2 = new QPieSlice("red", 3);
    slice2->setLabelVisible(true);
    slice2->setColor(slice2->label());
    series->append(slice2);
    QPieSlice* slice3 = new QPieSlice("blue", 3);
    slice3->setLabelVisible(true);
    slice3->setColor(slice3->label());
    slice3->setExploded(true);
    {
        QPieSlice* subSlice31 = new QPieSlice("cyan", 4);
        subSlice31->setLabelVisible(true);
        subSlice31->setColor(subSlice31->label());
        slice3->append(subSlice31);
        QPieSlice* subSlice32 = new QPieSlice("green", 1);
        subSlice32->setLabelVisible(true);
        subSlice32->setColor(subSlice32->label());
        slice3->append(subSlice32);
    }
    series->append(slice3);
    QPieSlice* slice4 = new QPieSlice("purple", 1);
    slice4->setLabelVisible(true);
    slice4->setColor(slice4->label());
    series->append(slice4);
    viewer.rootContext()->setContextProperty("cppPieSeries", series);

    viewer.setSource(QUrl("qrc:/qml/qmlsubpieslices/main.qml"));
    viewer.setResizeMode(QQuickView::SizeRootObjectToView);
    viewer.show();

    return app.exec();
}

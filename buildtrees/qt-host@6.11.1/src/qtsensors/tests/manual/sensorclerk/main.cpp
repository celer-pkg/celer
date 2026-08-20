// Copyright (C) 2017 Lorn Potter.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml/qqml.h>
#include <QDebug>


#include "collector.h"

int main( int argc, char** argv )
{
    QGuiApplication app( argc, argv );
    qmlRegisterType<Collector>("Collector", 1, 0, "Collector");
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:qml/main.qml"));
    view.show();
    return app.exec();
}


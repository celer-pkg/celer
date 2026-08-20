// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QQmlContext>
#include <QQuickView>

void registerContextProperties(QQuickView &view)
{
    view.rootContext()->setContextProperty(



                                        QLatin1StringView
                (  "myContextProperty3")         ,
                                           QDateTime::currentDateTime())
            ;
    view.rootContext()->setContextProperty(     QString
                                                (    "myContextProperty4"),
                                           QDateTime::currentDateTime());

    view.rootContext()->setContextProperty(QString("myContextProperty4"),
                                           QDateTime::currentDateTime());
}

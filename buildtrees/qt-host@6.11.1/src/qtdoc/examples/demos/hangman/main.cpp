// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtQuick>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include "hangmangame.h"
#include "purchasing/qmltypes/inappstoreqmltype.h"
#include "purchasing/inapp/inappproduct.h"
#include "purchasing/inapp/inapptransaction.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("QtProject");
    QCoreApplication::setApplicationName("Hangman");

    QQmlApplicationEngine engine(QUrl("qrc:/main.qml"));

    return app.exec();
}

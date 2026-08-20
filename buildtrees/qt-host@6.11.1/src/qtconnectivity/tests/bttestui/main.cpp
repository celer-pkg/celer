// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickView>
#include <QtQml/qqml.h>

#include <QtCore/QLoggingCategory>

#if QT_CONFIG(permissions)
#include <QtCore/qpermissions.h>
#endif

#include "btlocaldevice.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    QGuiApplication app(argc, argv);

#if QT_CONFIG(permissions)
    // Check Bluetooth permission and request it if the app doesn't have it
    auto permissionStatus = app.checkPermission(QBluetoothPermission{});
    if (permissionStatus == Qt::PermissionStatus::Undetermined) {
        app.requestPermission(QBluetoothPermission{},
                              &app, [&permissionStatus](const QPermission &permission) {
            qApp->exit(); // Exit the permission request processing started below
            permissionStatus = permission.status();
        });
        // Process permission request
        app.exec();
    }
    if (permissionStatus == Qt::PermissionStatus::Denied) {
        qWarning("Bluetooth permission denied, exiting");
        return -1;
    }
#endif

    qmlRegisterType<BtLocalDevice>("Local", 6, 5, "BluetoothDevice");

    QQuickView view;
    view.setSource(QUrl{u"qrc:///main.qml"_s});
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));
    view.show();

    return QGuiApplication::exec();
}

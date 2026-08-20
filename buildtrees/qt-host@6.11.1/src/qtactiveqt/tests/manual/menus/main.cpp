// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "menus.h"
#include <QApplication>
#include <QAxFactory>
#include <QScopedPointer>

QAXFACTORY_BEGIN(
    "{ce947ee3-0403-4fdc-895a-4fe779394b46}", // type library ID
    "{8de435ce-8d2a-46ac-b3b3-cb800d0847c7}") // application ID
    QAXCLASS(QMenus)
QAXFACTORY_END()

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QScopedPointer<QWidget> window;

    if (!QAxFactory::isServer()) {
        window.reset(new QMenus());
        window->show();
    }

    return a.exec();
}

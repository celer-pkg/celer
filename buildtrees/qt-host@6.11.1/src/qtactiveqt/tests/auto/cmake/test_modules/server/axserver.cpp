// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Stephen Kelly <stephen.kelly@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qt_windows.h>
#include <QAxFactory>
#include <QtAxServer/QAxFactory>

QT_BEGIN_NAMESPACE
QAxFactory *qax_instantiate()
{
    return 0;
}
QT_END_NAMESPACE

int main(int argc, char **argv)
{
    QAxFactory::isServer();
    return 0;
}

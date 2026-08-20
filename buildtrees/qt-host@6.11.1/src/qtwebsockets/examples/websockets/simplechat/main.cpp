// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#include <QtCore/QCoreApplication>

#include "chatserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    ChatServer server(1234);

    return a.exec();
}

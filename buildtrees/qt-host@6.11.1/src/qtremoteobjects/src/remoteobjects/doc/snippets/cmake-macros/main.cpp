// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QCoreApplication>
#include "simpleswitch.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SimpleSwitch srcSwitch; // create simple switch

    QRemoteObjectHost srcNode(QUrl(QStringLiteral("local:replica"))); // create host node without Registry
    srcNode.enableRemoting(&srcSwitch); // enable remoting/Sharing

    return a.exec();
}

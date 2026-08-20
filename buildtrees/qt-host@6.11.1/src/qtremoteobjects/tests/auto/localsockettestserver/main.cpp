// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QRemoteObjectNode>

#include "../../shared/testutils.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QObject remotedObject;
    remotedObject.setObjectName(QStringLiteral("connectme"));
    QRemoteObjectHost node(QUrl(QStringLiteral(LOCAL_SOCKET ":crashMe")));
    node.enableRemoting(&remotedObject);

    return a.exec();
}

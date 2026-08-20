// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qdebug.h>

#include "mytestserver.h"

MyTestServer::MyTestServer(QObject *parent)
    : ParentClassSimpleSource(parent)
{
    qDebug() << "Server started";
}

MyTestServer::~MyTestServer()
{
    qDebug() << "Server stopped";
}

bool MyTestServer::start()
{
    setStarted(true);
    return true;
}

bool MyTestServer::quit()
{
    emit quitApp();
    return true;
}

ParentClassSource::MyEnum MyTestServer::enumSlot(QPoint p, MyEnum myEnum)
{
    Q_UNUSED(p)
    Q_UNUSED(myEnum)
    return ParentClassSource::foobar;
}

Qt::DateFormat MyTestServer::dateSlot(Qt::DateFormat date)
{
    Q_UNUSED(date)
    return Qt::RFC2822Date;
}

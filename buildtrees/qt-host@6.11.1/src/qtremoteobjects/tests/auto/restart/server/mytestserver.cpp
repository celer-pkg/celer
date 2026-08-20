// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <qdebug.h>

#include "mytestserver.h"
#include "rep_subclass_source.h"

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

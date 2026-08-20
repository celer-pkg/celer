// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MYTESTSERVER_H
#define MYTESTSERVER_H

#include <QTimer>

#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QtRemoteObjects/qremoteobjectsource.h>

#include "rep_subclass_source.h"

class MyTestServer : public ParentClassSimpleSource
{
    Q_OBJECT

public:
    MyTestServer(QObject *parent = nullptr);
    ~MyTestServer() override;

public Q_SLOTS:
    bool start() override;
    bool quit() override;

Q_SIGNALS:
    void quitApp();
};

#endif // MYTESTSERVER_H

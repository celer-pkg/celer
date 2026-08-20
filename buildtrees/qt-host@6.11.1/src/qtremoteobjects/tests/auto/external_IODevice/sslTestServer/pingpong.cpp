// Copyright (C) 2018 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "pingpong.h"

PingPong::PingPong(QObject *parent)
    : PingPongSimpleSource(parent)
{}

void PingPong::ping(const QString &message)
{
    emit pong("Pong " + message);
}

void PingPong::quit()
{
    // Kill me softly
    QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

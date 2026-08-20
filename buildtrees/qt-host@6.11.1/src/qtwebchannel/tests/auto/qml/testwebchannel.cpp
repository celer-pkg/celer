// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testwebchannel.h"

#include <QtWebChannel/private/qwebchannel_p.h>
#include <QtWebChannel/private/qmetaobjectpublisher_p.h>

QT_BEGIN_NAMESPACE

TestWebChannel::TestWebChannel(QObject *parent)
    : QQmlWebChannel(parent)
{

}

TestWebChannel::~TestWebChannel()
{

}

bool TestWebChannel::clientIsIdle() const
{
    for (auto *transport : QWebChannel::d_func()->transports) {
        if (QWebChannel::d_func()->publisher->isClientIdle(transport))
            return true;
    }
    return false;
}

QT_END_NAMESPACE
